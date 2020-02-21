#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include "Camera.h"

class Portal;

class Portal
{
public:
    bool open = false;
    void draw(MatrixStack &M);
    void linkPortal(Portal *other);
    void setPosition(glm::vec3 position, glm::quat orientation);
    void updateCamera(const Camera &playerCamera);
    void drawOutline(MatrixStack &M);
    glm::mat4 modifyProjectionMatrix(const glm::mat4 &P, const glm::mat4 &V);

    glm::vec3 getUp();
    glm::vec3 getForward();
    std::string model = "portal";
    Camera camera;
    Portal *linkedPortal;
    glm::vec3 localForward = glm::vec3(0, 1, 0);
    glm::vec3 localUp = glm::vec3(0, 0, -1);
private:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color = glm::vec3(1, 1, 1);
    float outlineWidth = 0;
    glm::quat orientation;
};