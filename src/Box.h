#pragma once

#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>
#include "MatrixStack.h"

class Box
{
public:
    void init(physx::PxVec3 location, physx::PxVec3 scale, physx::PxQuat rotation);
    void draw(MatrixStack &M);
    float density = 10.0f;
    physx::PxRigidDynamic *body;
private:
    glm::vec3 scale;
};