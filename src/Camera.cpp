#include "Camera.h"

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <memory>
#include <iostream>

using namespace std;
using namespace glm;

void Camera::update(float dt)
{
    // Common mouse movement
    double xpos, ypos;
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glfwGetCursorPos(windowManager->getHandle(), &xpos, &ypos);
    double dx = xpos - prevCursorPosX;
    double dy = -(ypos - prevCursorPosY);
    prevCursorPosX = xpos;
    prevCursorPosY = ypos;
    double radPerPx = M_PI / height;
    yaw += dx * radPerPx;
    pitch = std::max(std::min(pitch + dy * radPerPx, radians(80.0)), -radians(80.0));

    if (mode == FREE_CAM)
    {
        // Translational movement
        float speed = flyingSpeed;
        vec3 velocity = vec3(0, 0, 0);
        vec3 strafe = normalize(cross(lookAtPoint - eye, upVec));
        vec3 dolly = normalize(lookAtPoint - eye);
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(windowManager->getHandle(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speed *= sprintFactor;
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_W) == GLFW_PRESS)
        {
            velocity += dolly;
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_S) == GLFW_PRESS)
        {
            velocity -= dolly;
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_D) == GLFW_PRESS)
        {
            velocity += strafe;
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_A) == GLFW_PRESS)
        {
            velocity -= strafe;
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_E) == GLFW_PRESS)
        {
            velocity += vec3(0, 1, 0);
        }
        if (glfwGetKey(windowManager->getHandle(), GLFW_KEY_Q) == GLFW_PRESS)
        {
            velocity -= vec3(0, 1, 0);
        }
        if (length(velocity) != 0 && length(velocity) != NAN)
        {
            eye += normalize(velocity) * speed * dt;
        }

        // Mouse movement
        lookAtPoint.x = eye.x + cos(pitch) * sin(yaw);
		lookAtPoint.y = eye.y + sin(pitch);
		lookAtPoint.z = eye.z + cos(pitch) * cos(M_PI - yaw);
    }
    else if (mode == FOLLOW_CAM)
    {
        // nothing
    }
}

void Camera::lookAt(shared_ptr<MatrixStack> V)
{
    if (mode == FREE_CAM)
    {
        V->lookAt(eye, lookAtPoint, upVec);
    }
    else if (mode == FOLLOW_CAM)
    {
        // nothing
    }
}

void Camera::init(WindowManager *windowManager, vec3 pos, vec3 lookDir)
{
    this->windowManager = windowManager;
    glfwGetCursorPos(windowManager->getHandle(), &prevCursorPosX, &prevCursorPosY);
    eye = pos;
    lookAtPoint = pos + lookDir;
}

void Camera::toggleMode()
{
    if (mode == FREE_CAM)
    {
        mode = FOLLOW_CAM;
    }
    else
    {
        mode = FREE_CAM;
    }
}

void Camera::scrollCallback(double deltaY)
{
    radius = std::max(std::min(radius - deltaY, 50.0), 1.0);
}