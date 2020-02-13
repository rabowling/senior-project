#pragma once

#include "WindowManager.h"
#include "MatrixStack.h"
#include <memory>
#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>


class Camera
{
public:
    enum CameraMode { FREE_CAM, FOLLOW_CAM };
    void lookAt(std::shared_ptr<MatrixStack> V);
    void update(float dt);
    void init(glm::vec3 pos, glm::vec3 lookDir);
    void toggleMode();
    void scrollCallback(double deltaY);

    glm::vec3 eye;
    double radius = 10;
    glm::vec3 lookAtPoint;
    glm::vec3 upVec = glm::vec3(0, 1, 0);
    
private:
    WindowManager *windowManager;
    double pitch = 0;
    double yaw = 0;
    CameraMode mode = FREE_CAM;
    physx::PxController *mController;

    // When in free cam
    float flyingSpeed = 5;
    float sprintFactor = 3;
};