#pragma once

#include "GameObject.h"
#include "Shape.h"
#include "Material.h"
#include "MatrixStack.h"
#include "Portal.h"
#include <glm/glm.hpp>

class Portal;

class PortalOutline : public GameObject
{
public:
    PortalOutline(glm::vec3 color, std::string model);
    void draw(MatrixStack &M);
    Portal *parent = nullptr;

    virtual Shape *getModel() const;
    virtual glm::mat4 getTransform() const;
    virtual Material *getMaterial() const;
    glm::vec3 color;
private:
    std::string model;
};