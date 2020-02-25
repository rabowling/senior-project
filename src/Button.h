#pragma once

#include <PxPhysicsAPI.h>
#include "MatrixStack.h"

class Button
{
public:
    void onContact(physx::PxActor *other);
    void init(physx::PxVec3 location);
    void draw(MatrixStack &M);
    physx::PxRigidStatic *body;
    bool pressed = false;
};
