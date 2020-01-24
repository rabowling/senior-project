#include "Player.h"
#include <iostream>
#include "Utils.h"
#include "Application.h"

using namespace std;
using namespace physx;

void Player::init() {
    PxCapsuleControllerDesc desc;

    // We can tweak these as necessary
    desc.radius = 1.0f;
    desc.height = 3.0f;
    desc.position = PxExtendedVec3(0,4,0);
    desc.material = app.physics.getPhysics()->createMaterial(0.5f,0.5f,0.6f);

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
    if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_W) == GLFW_PRESS) {
        fSpeed += mWalkSpeed;
    }
    if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_S) == GLFW_PRESS) {
        fSpeed -= mWalkSpeed;
    }
    if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_D) == GLFW_PRESS) {
        sSpeed += mWalkSpeed;
    }
    if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_A) == GLFW_PRESS) {
        sSpeed -= mWalkSpeed;
    }

    // Move character controller
    direction = ((direction * fSpeed) + (right * sSpeed)) * dt;
    direction.y += gravity * dt;
    mController->move(direction, 0.0f, dt, NULL, NULL);

    // Check for picking up item
    bool pressedE = false;
    if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_E) == GLFW_PRESS && !pressingE) {
        pressingE = true;
        pressedE = true;
    }
    else if (glfwGetKey(mWindowManager->getHandle(), GLFW_KEY_E) == GLFW_RELEASE && pressingE) {
        pressingE = false;
    }

    if (pressedE) {
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

