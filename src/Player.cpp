#include "Player.h"

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
}

void Player::update(float dt) {
    glm::vec3 lookAt = mCamera->lookAtPoint;
    PxVec3 direction = PxExtendedVec3(lookAt.x, lookAt.z, lookAt.z) - mController->getPosition();
    direction.y = 0.0f;
    PxVec3 up = PxVec3(0.0f,1.0f,0.0f);
    PxVec3 right = direction.cross(up);
    direction.normalize();
    right.normalize();

    float fSpeed = 0.0f;
    float sSpeed = 0.0f;
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

    direction = ((direction * fSpeed) + (right * sSpeed)) * dt;
    mController->move(direction, 0.0f, dt, NULL, NULL);
}

