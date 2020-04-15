#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include "Camera.h"
#include "GameObject.h"
#include "PortalOutline.h"

class Portal;
class PortalOutline;

class Portal : public GameObject
{
public:
    Portal(glm::vec3 position, glm::vec3 scale, glm::quat orientation, std::string model);
    bool open = true;
    void draw(MatrixStack &M);
    void linkPortal(Portal *other);
    void setPosition(glm::vec3 position, glm::quat orientation);
    void updateCamera(const Camera &playerCamera);
    void drawOutline(MatrixStack &M);
    glm::mat4 modifyProjectionMatrix(const glm::mat4 &P, const glm::mat4 &V);
    bool facing(const glm::vec3 &point);

    virtual Shape *getModel() const;
    virtual glm::mat4 getTransform() const;
    virtual Material *getMaterial() const;

    glm::vec3 getUp();
    glm::vec3 getForward();
    std::string model;
    void setOutline(PortalOutline *outline);
    PortalOutline *outline;
    bool hasOutline = false;

    Camera camera;
    Portal *linkedPortal = nullptr;
    glm::vec3 localForward = glm::vec3(0, 1, 0);
    glm::vec3 localUp = glm::vec3(0, 0, -1);
    glm::vec3 cachedForward;
    bool isForwardCached = false;
    glm::vec3 cachedUp;
    bool isUpCached = false;
    
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale = glm::vec3(1);

    float intensity;
};