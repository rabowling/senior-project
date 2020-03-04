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
    camera.init(px2glm(desc.position), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));

    // Create and link player-controlled portals
    app.portals.push_back(Portal(vec3(0), vec3(1), quat(1, 0, 0, 0), "portal"));
    app.portals.push_back(Portal(vec3(0), vec3(1), quat(1, 0, 0, 0), "portal"));
    auto it = app.portals.rbegin();
    portals[1] = &(*it);
    portals[0] = &(*next(it));
    portals[0]->linkPortal(portals[1]);

    portals[0]->setOutline(new PortalOutline(vec3(0, 0, 1), "portal_outline"));
    portals[1]->setOutline(new PortalOutline(vec3(1, 0.5, 0), "portal_outline"));
}

void Player::update(float dt) {
    camera.update(px2glm(mController->getPosition()) + glm::vec3(0, height / 2, 0),
        app.controls.getMouseDeltaX(), app.controls.getMouseDeltaY());

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
        if (app.controls.isPressed(Controls::JUMP)) {
            velocity.y = jumpSpeed;
        }
    }
    else {
        velocity.y += GRAVITY * dt;
    }
    displacement += velocity * dt;
    mController->move(displacement, 0.01f, dt, NULL, NULL);

    // Check for picking up item
    if (app.controls.isPressed(Controls::USE)) {
        if (!heldItem) {
            // Calculate origin and direction vectors
            origin = glm2px(camera.eye);
            unitDir = glm2px(camera.lookAtPoint - camera.eye);
            unitDir.normalize();
            origin += unitDir;

            // Perform raycast
            PxRaycastBuffer hit;
            PxReal maxDist = 50.0;
            raycastMode = PICK_UP;
            bool success = app.physics.getScene()->raycast(origin, unitDir, maxDist, hit, PxHitFlags(PxHitFlag::eDEFAULT),
                PxQueryFilterData(PxQueryFlags(PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER)), this);
            if (success) {
                if (hit.block.actor->is<PxRigidBody>()) {
                    heldItem = static_cast<PxRigidBody *>(hit.block.actor);
                    heldItem->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
                }
            }
        }
        else {
            heldItem->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
            heldItem = NULL;
        }
    }

    // Move held item in front of player
    if (heldItem != NULL) {
        float holdDistance = 5.f;
        PxVec3 targetLocation = glm2px(camera.eye + holdDistance * (camera.lookAtPoint - camera.eye));
        PxVec3 vectorToTargetLocation = targetLocation - heldItem->getGlobalPose().p;

        heldItem->addForce(vectorToTargetLocation * 10.0f - heldItem->getLinearVelocity(), PxForceMode::eVELOCITY_CHANGE, true);
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
            PxQueryFilterData(PxQueryFlags(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::ePREFILTER)), this);
        if (success) {
            Portal *portal = app.controls.isPressed(Controls::PRIMARY_FIRE) ? portals[0] : portals[1];
            vec3 newPos = px2glm(hit.block.position);
            vec3 cameraRight = cross(px2glm(unitDir), camera.upVec);
            vec3 portalUp = cross(px2glm(hit.block.normal), cameraRight);
            quat newRot = quatLookAt(px2glm(hit.block.normal), portalUp)
                * angleAxis((float) M_PI_2, cross(portal->localForward, portal->localUp));
            portal->setPosition(newPos, newRot);
        }
    }
}

// Filter raycast
PxQueryHitType::Enum Player::preFilter(const PxFilterData &filterData, const PxShape *shape, const PxRigidActor *actor, PxHitFlags &queryFlags)
{
    if (raycastMode == PICK_UP) {
        if (mController->getActor() == actor) {
            return PxQueryHitType::eNONE;
        }
    }
    else if (raycastMode == FIRE_PORTAL) {
        if (mController->getActor() == actor) {
            return PxQueryHitType::eNONE;
        }
    }
    
    return PxQueryHitType::eBLOCK;
}