#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <memory>
#include <iostream>
#include "Application.h"

using namespace std;
using namespace glm;

void Camera::update(glm::vec3 pos, float deltaYaw, float deltaPitch)
{
    yaw -= deltaYaw;
    pitch = std::max(std::min(pitch + deltaPitch, radians(80.0)), -radians(80.0));
    
    eye.x = pos.x;
    eye.y = pos.y;
    eye.z = pos.z;

    lookAtPoint.x = eye.x + cos(pitch) * sin(yaw);
	lookAtPoint.y = eye.y + sin(pitch);
	lookAtPoint.z = eye.z + cos(pitch) * cos(M_PI - yaw);
}

glm::mat4 Camera::getLookAt() {
    return glm::lookAt(eye, lookAtPoint, upVec);
}

void Camera::init(glm::vec3 pos, glm::vec3 lookDir, glm::vec3 upVec)
{
    eye = pos;
    lookAtPoint = pos + lookDir;
    pitch = asin(-lookDir.y);
    yaw = atan2(lookDir.x, lookDir.z);
    this->upVec = upVec;
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
