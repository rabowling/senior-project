#include "Player.h"
#include <iostream>
#include "Utils.h"
#include "Application.h"
#include "Controls.h"
#include "Physics.h"
#include <PxPhysicsAPI.h>
#include <cmath>
#include <glm/glm.hpp>

using namespace std;
using namespace physx;
using namespace glm;

void Player::onShapeHit(const PxControllerShapeHit &hit) {
    if (hit.actor->is<PxRigidDynamic>()) {
        // Push boxes
        PxRigidDynamic *actor = static_cast<PxRigidDynamic *>(hit.actor);
        PxVec3 point(hit.worldPos.x, hit.worldPos.y, hit.worldPos.z);
        PxVec3 impulse = hit.dir;
        PxVec3 deltaLinearVelocity;
        PxVec3 deltaAngularVelocity;
        PxRigidBodyExt::computeVelocityDeltaFromImpulse(*actor, actor->getGlobalPose(),
            point, impulse, actor->getInvMass(), 1, deltaLinearVelocity, deltaAngularVelocity);
        deltaLinearVelocity.y = 0;
        deltaAngularVelocity.z = 0;
        deltaAngularVelocity.x = 0;
        actor->addForce(deltaLinearVelocity * 1000, PxForceMode::eVELOCITY_CHANGE);
        actor->addTorque(deltaAngularVelocity * 10, PxForceMode::eVELOCITY_CHANGE);
    }
    for (int i = 0; i < 3; i++) {
        if (velocity[i] > 0 && hit.worldNormal[i] < 0) {
            velocity[i] *= 1 + hit.worldNormal[i];
        }
        else if (velocity[i] < 0 && hit.worldNormal[i] > 0) {
            velocity[i] *= 1 - hit.worldNormal[i];
        }

    }
}

void Player::setPosition(float x, float y, float z) {
    mController->setFootPosition(PxExtendedVec3(x, y, z));
}

void Player::init() {
    PxCapsuleControllerDesc desc;

    // We can tweak these as necessary
    desc.radius = radius;
    desc.height = height;
    desc.position = PxExtendedVec3(20,4,45);
    desc.material = app.physics.getPhysics()->createMaterial(0.3f,0.3f,0.3f);
    desc.reportCallback = this;

    mController = app.physics.getControllerManager()->createController(desc);
    velocity = PxVec3(0);
    camera.init(px2glm(desc.position), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

    raycastFilterCallback = make_unique<RaycastFilterCallback>(this);
    moveFilterCallback = make_unique<MoveFilterCallback>(this);

    // Create and link player-controlled portals
    app.portals.push_back(Portal(vec3(0), vec3(1), quat(1, 0, 0, 0), "portal"));
    app.portals.push_back(Portal(vec3(0), vec3(1), quat(1, 0, 0, 0), "portal"));
    auto it = app.portals.rbegin();
    portals[1] = &(*it);
    portals[0] = &(*next(it));
    portals[0]->linkPortal(portals[1]);
    portals[0]->open = false;
    portals[1]->open = false;

    portals[0]->setOutline(new PortalOutline(vec3(0, 0, 1), "portal_outline"));
    portals[1]->setOutline(new PortalOutline(vec3(1, 0.5, 0), "portal_outline"));
}

void Player::update(float dt) {
    camera.update(px2glm(mController->getPosition()) + camOffset, app.controls.getMouseDeltaX(), app.controls.getMouseDeltaY());

    PxVec3 direction = glm2px(camera.lookAtPoint - camera.eye);
    direction.y = 0.0f;
    PxVec3 up = glm2px(camera.upVec);
    PxVec3 right = direction.cross(up);
    direction.normalize();
    right.normalize();

    float fSpeed = 0.0f; // forward speed
    float sSpeed = 0.0f; // sideways speed
    if (app.controls.isHeld(Controls::FORWARD)) {
        fSpeed += mWalkSpeed;
    }
    if (app.controls.isHeld(Controls::BACKWARD)) {
        fSpeed -= mWalkSpeed;
    }
    if (app.controls.isHeld(Controls::RIGHT)) {
        sSpeed += mWalkSpeed;
    }
    if (app.controls.isHeld(Controls::LEFT)) {
        sSpeed -= mWalkSpeed;
    }
    if (app.controls.isHeld(Controls::SPRINT)) {
        fSpeed *= 2.0f;
        sSpeed *= 2.0f;
    }

    // Move character controller
    PxVec3 displacement = ((direction * fSpeed) + (right * sSpeed)) * dt;

    // Handle jumping
    PxControllerState state;
    mController->getState(state);
    if (state.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) {
        velocity *= 0.9f;
        if (app.controls.isHeld(Controls::JUMP)) {
            velocity.y = jumpSpeed;
        }
    }
    else {
        velocity.y += GRAVITY * dt;
    }
    displacement += velocity * dt;

    // filter out objects when moving through portal
    PxControllerFilters moveFilter;
    moveFilter.mFilterCallback = moveFilterCallback.get();
    moveFilter.mFilterFlags = PxQueryFlags(PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER);

    mController->move(displacement, 0.01f, dt, moveFilter, NULL);

    if (mController->getPosition().y <= -50) {
        cout << "respawn player" << endl;
        resetLevel();
    }
    
    // keep track of portals that player is touching
    prevTouchingPortals = touchingPortals;
    touchingPortals.clear();
    for (Portal &portal : app.portals) {
        if (portal.pointInBounds(px2glm(mController->getPosition()))) {
            touchingPortals.push_back(&portal);
        }
    }

    // teleport player if they move through the portal
    for (Portal *portal : prevTouchingPortals) {
        if (!portal->facing(px2glm(mController->getPosition()))) {

            MatrixStack camTransform;
            camTransform.translate(portal->linkedPortal->position);
            camTransform.rotate(M_PI, portal->linkedPortal->getUp());
            camTransform.rotate(portal->linkedPortal->orientation);
            camTransform.rotate(inverse(portal->orientation));
            camTransform.translate(-portal->position);

            vec3 newPos = vec3(camTransform.topMatrix() * vec4(px2glm(mController->getPosition()), 1));
            mController->setPosition(glm2pxex(newPos));

            vec3 newDir = vec3(camTransform.topMatrix() * vec4(camera.lookAtPoint - camera.eye, 0));
            camera.init(newPos + camOffset, newDir, camera.upVec);

            vec3 newVel = vec3(camTransform.topMatrix() * vec4(px2glm(velocity), 0));
            velocity = glm2px(newVel);

            holdTransform = inverse(camTransform.topMatrix()) * holdTransform;

            prevTouchingPortals = touchingPortals;
            touchingPortals.clear();
            for (Portal &portal : app.portals) {
                if (portal.pointInBounds(px2glm(mController->getPosition()))) {
                    touchingPortals.push_back(&portal);
                }
            }
            break;
        }
    }

    origin = glm2px(camera.eye);
    unitDir = glm2px(camera.lookAtPoint - camera.eye);
    unitDir.normalize();
    origin += unitDir;

    // Perform raycast
    PxRaycastBuffer hit;
    PxReal maxDist = 50.0;
    raycastMode = USE;

    if (!heldItem) {
        bool success = app.physics.getScene()->raycast(origin, unitDir, maxDist, hit, PxHitFlags(PxHitFlag::eDEFAULT),
            PxQueryFilterData(PxQueryFlags(PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER)), raycastFilterCallback.get());
        if (success) {
            GameObject *obj = static_cast<GameObject *>(hit.block.actor->userData);
            if (dynamic_cast<Box *>(obj)) {
                //app.hud.updateToolTip("Pick up.");
                if (app.controls.isPressed(Controls::USE)) {
                    holdTransform = mat4(1);
                    heldItem = static_cast<PxRigidBody *>(hit.block.actor);
                    heldItem->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
                }
            } else if (dynamic_cast<LightSwitch *>(obj)) {
                //cout << "pointing at light switch" << endl;
                //app.hud.updateToolTip("Turn on light.");
                if (app.controls.isPressed(Controls::USE)) {
                    LightSwitch *lswitch = static_cast<LightSwitch *>(obj);
                    for (Light l : app.lights) {
                        if (l.id == lswitch->lightId) {
                            app.currentLight = l;
                            cout << "new light: " << l.id << endl;
                        }
                    }
                }
            } else {
                //app.hud.updateToolTip("");
            }
        }
    } else {
        float holdDistance = 5.f;
        vec3 targetLocation = camera.eye + holdDistance * (camera.lookAtPoint - camera.eye);
        targetLocation = vec3(holdTransform * vec4(targetLocation, 1));
        PxVec3 vectorToTargetLocation = glm2px(targetLocation) - heldItem->getGlobalPose().p;
        //app.hud.updateToolTip("Drop.");

        heldItem->addForce(vectorToTargetLocation * 10.0f - heldItem->getLinearVelocity(), PxForceMode::eVELOCITY_CHANGE, true);
        if (app.controls.isPressed(Controls::USE)) {
            heldItem->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
            heldItem = NULL;
        }
    }

    // Fire portals
    if (app.controls.isPressed(Controls::PRIMARY_FIRE) || app.controls.isPressed(Controls::SECONDARY_FIRE)) {
        origin = glm2px(camera.eye);
        unitDir = glm2px(camera.lookAtPoint - camera.eye);
        unitDir.normalize();
        origin += unitDir;

        PxRaycastBuffer hit;
        PxReal maxDist = 100.0;
        raycastMode = FIRE_PORTAL;
        bool success = app.physics.getScene()->raycast(origin, unitDir, maxDist, hit, PxHitFlags(PxHitFlag::eDEFAULT),
            PxQueryFilterData(PxQueryFlags(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER)), raycastFilterCallback.get());
        if (success) {
            Portal *portal = app.controls.isPressed(Controls::PRIMARY_FIRE) ? portals[0] : portals[1];
            vec3 newPos = px2glm(hit.block.position);
            vec3 cameraRight = cross(px2glm(unitDir), camera.upVec);
            vec3 portalUp = cross(px2glm(hit.block.normal), cameraRight);
            quat newRot = quatLookAt(px2glm(hit.block.normal), portalUp)
                * angleAxis((float) M_PI_2, cross(portal->localForward, portal->localUp));
            portal->setPosition(newPos, newRot);
            portal->open = true;

            PortalLight &curLight = (app.controls.isPressed(Controls::PRIMARY_FIRE)) ? app.portalLights[BLUE_PORTAL] : app.portalLights[ORANGE_PORTAL];
            curLight.direction = portal->getForward();
            curLight.position = newPos;
            curLight.portal = portal;

            //cout << curLight.direction.x << ", " << curLight.direction.y << ", " << curLight.direction.z << endl;
            //cout << curLight.position.x << ", " << curLight.position.y << ", " << curLight.position.z << endl;

            portal->surface = hit.block.actor;
        }
    }
}

void Player::resetLevel() {
    setPosition(startPos.x, startPos.y, startPos.z);
    app.currentLight = app.lights[0];
    for (Box b : app.boxes) {
        b.respawn();
    }
}

Player::RaycastFilterCallback::RaycastFilterCallback(Player *parent) : parent(parent) {

}


PxQueryHitType::Enum Player::RaycastFilterCallback::preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
{
    if (parent->raycastMode == USE) {
        if (parent->mController->getActor() == actor) {
            return PxQueryHitType::eNONE;
        }
    }
    else if (parent->raycastMode == FIRE_PORTAL) {
        if (parent->mController->getActor() == actor) {
            return PxQueryHitType::eNONE;
        }
    }
    
    return PxQueryHitType::eBLOCK;
}

Player::MoveFilterCallback::MoveFilterCallback(Player *parent) : parent(parent) {

}

PxQueryHitType::Enum Player::MoveFilterCallback::preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
{
    PxBounds3 bbox = PxShapeExt::getWorldBounds(*shape, *actor, 1);
    for (Portal &portal : app.portals) {
        if (portal.pointInBounds(px2glm(parent->mController->getPosition()))
                && (portal.surface == actor || (!portal.facing(px2glm(bbox.maximum)) && !portal.facing(px2glm(bbox.minimum))))) {
            return PxQueryHitType::eNONE;
        }
    }
    return PxQueryHitType::eBLOCK;
}

void Player::draw(MatrixStack &M) {
    M.pushMatrix();
    M.translate(px2glm(mController->getPosition()) + glm::vec3(0, height / 2, 0));
    M.rotate(camera.yaw + M_PI, vec3(0, -1, 0));
    M.rotate(camera.pitch, vec3(-1, 0, 0));
    M.scale(0.01);
    if (!app.renderingCubemap) {
        app.materialManager.bind("player");
    }
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw("connor");
    M.popMatrix();
}
