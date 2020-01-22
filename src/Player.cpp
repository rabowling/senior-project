#include "Player.h"
#include <iostream>

using namespace std;
using namespace physx;

Player::Player(WindowManager *windowManager, Physics *physics, Camera *camera) {
    PxCapsuleControllerDesc desc;

    // We can tweak these as necessary
    desc.radius = 1.0f;
    desc.height = 3.0f;
    desc.position = PxExtendedVec3(0,4,0);
    desc.material = physics->getPhysics()->createMaterial(0.5f,0.5f,0.6f);

    mController = physics->getControllerManager()->createController(desc);
    mWindowManager = windowManager;
    velocity = PxVec3(0);
    mCamera = camera;
    mScene = physics->getScene();
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
    mController->move(direction, 0.0f, dt, NULL, NULL);

    // Check for mouse down
    if (glfwGetMouseButton(mWindowManager->getHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isCasting) {
        PxRaycastBuffer hit;
        bool status;
        PxExtendedVec3 pos;
        PxVec3 *upForce = new PxVec3(0, 10.0f, 0);
        PxReal maxDist = 50.0;

        isCasting = true; // debounce variable

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
                currentTarget = static_cast<PxRigidBody *>(hit.block.actor);
                lastLocation = hit.block.position;
            }
        }
    }

    // Check for mouse up
    if (glfwGetMouseButton(mWindowManager->getHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && isCasting) {
        isCasting = false;
        currentTarget = NULL;
    }

    // Pick up block if currentTarget is not null (in progress)
    if (currentTarget != NULL) {
        PxVec3 targetLocation = origin + (5.0f * unitDir);
        PxVec3 vectorToTargetLocation = targetLocation - lastLocation;
        vectorToTargetLocation.normalize();
        currentTarget->addForce(vectorToTargetLocation * 5.0f, PxForceMode::eVELOCITY_CHANGE, true);
        lastLocation += vectorToTargetLocation * 5.0f * dt;
    }
}

