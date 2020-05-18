#pragma once

#include <PxPhysicsAPI.h>
#include "MatrixStack.h"
#include "GameObject.h"

class Door;
class Button : public GameObject
{
public:
    void onContact(physx::PxActor *other);
    void init(physx::PxVec3 location);
    void draw(MatrixStack &M);
    physx::PxRigidStatic *body;
    bool pressed = false;
    virtual Shape *getModel() const;
    virtual glm::mat4 getTransform() const;
    virtual Material *getMaterial() const;
    Door *linkedDoor = nullptr;
};
