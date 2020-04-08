#pragma once

#include "WindowManager.h"
#include "MatrixStack.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <PxPhysicsAPI.h>


class Camera
{
public:
    enum CameraMode { FREE_CAM, FOLLOW_CAM };
    glm::mat4 getLookAt();
    void update(glm::vec3 pos, float deltaPitch, float deltaYaw);
    void init(glm::vec3 pos, glm::vec3 lookDir, glm::vec3 upVec);
    void toggleMode();
    void scrollCallback(double deltaY);

    glm::vec3 eye;
    double radius = 10;
    glm::vec3 lookAtPoint;
    glm::vec3 upVec;
    
    double pitch = 0;
    double yaw = 0;
private:
    CameraMode mode = FREE_CAM;

    // When in free cam
    float flyingSpeed = 5;
    float sprintFactor = 3;
};