#pragma once

#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>
#include "MatrixStack.h"
#include "GameObject.h"

class Box : public GameObject
{
public:
    void init(physx::PxVec3 location, physx::PxVec3 scale, physx::PxQuat rotation);
    void draw(MatrixStack &M);
    float density = 10.0f;
    physx::PxRigidDynamic *body;
    
    virtual Shape *getModel() const;
    virtual glm::mat4 getTransform() const;
private:
    glm::vec3 scale;
};