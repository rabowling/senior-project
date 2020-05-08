#pragma once

#include "Shape.h"
#include <vector>
#include "Material.h"
#include <PxPhysicsAPI.h>

class GameObject
{
public:
    virtual Shape *getModel() const = 0;
    virtual Material *getMaterial() const = 0;
    virtual glm::mat4 getTransform() const = 0;

    virtual void onContactModify(const physx::PxRigidActor *actor, physx::PxContactSet &contacts) {}

    // cache transformed geometry to speed up raytracing
    virtual void cacheGeometry();
    virtual bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &u, float &v, float &d, unsigned int &faceIndex) const;
    std::vector<float> posBufCache;
    int cacheFrame = -1;
};