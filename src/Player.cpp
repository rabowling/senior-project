#include "Player.h"
#include <iostream>
#include "Utils.h"
#include "Application.h"
#include "Controls.h"
#include "Physics.h"
#include <PxPhysicsAPI.h>
#include <cmath>

using namespace std;
using namespace physx;

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

void Player::onControllerHit(const PxControllersHit &hit) {

}

void Player::onObstacleHit(const PxControllerObstacleHit &hit) {

}

void Player::init() {
    PxCapsuleControllerDesc desc;

    // We can tweak these as necessary
    desc.radius = 1.0f;
    desc.height = 3.0f;
    desc.position = PxExtendedVec3(0,4,0);
    desc.material = app.physics.getPhysics()->createMaterial(0.3f,0.3f,0.3f);
    desc.reportCallback = this;

    mController = app.physics.getControllerManager()->createController(desc);
    mWindowManager = &app.windowManager;
    velocity = PxVec3(0);
    mCamera = &app.camera;
    mScene = app.physics.getScene();
}

void Player::update(float dt) {
    glm::vec3 lookAt = mCamera->lookAtPoint;
    PxVec3 direction = PxExtendedVec3(lookAt.x, lookAt.y, lookAt.z) - mController->getPosition();
    direction.y = 0.0f;
    PxVec3 up = PxVec3(0.0f,1.0f,0.0f);
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
            PxRaycastBuffer hit;
            bool status;
            PxExtendedVec3 pos;
            PxVec3 *upForce = new PxVec3(0, 10.0f, 0);
            PxReal maxDist = 50.0;

            // Calculate origin and direction vectors
            pos = mController->getPosition();
            origin = PxVec3(pos.x, pos.y, pos.z);
            unitDir = PxExtendedVec3(lookAt.x, lookAt.y, lookAt.z) - mController->getPosition();
            unitDir.normalize();
            origin += unitDir;

            // Perform raycast
            status = mScene->raycast(origin, unitDir, maxDist, hit);
            if (status) {
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
        PxVec3 targetLocation = glm2px(mCamera->eye + holdDistance * (lookAt - mCamera->eye));
        PxVec3 vectorToTargetLocation = targetLocation - heldItem->getGlobalPose().p;

        heldItem->addForce(vectorToTargetLocation * 10.0f - heldItem->getLinearVelocity(), PxForceMode::eVELOCITY_CHANGE, true);
    }
}

