#pragma once

#include "Shape.h"

class GameObject
{
public:
    virtual Shape *getModel() const = 0;
    virtual glm::mat4 getTransform() const = 0;
};