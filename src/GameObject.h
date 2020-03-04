#pragma once

#include "Shape.h"
#include <vector>
#include "Material.h"

class GameObject
{
public:
    virtual Shape *getModel() const = 0;
    virtual Material *getMaterial() const = 0;
    virtual glm::mat4 getTransform() const = 0;

    // cache transformed geometry to speed up raytracing
    virtual void cacheGeometry();
    std::vector<float> posBufCache;
    int cacheFrame = -1;
};