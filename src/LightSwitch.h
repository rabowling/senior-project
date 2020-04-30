#pragma once

#include <PxPhysicsAPI.h>
#include "MatrixStack.h"
#include "GameObject.h"

class LightSwitch : public GameObject
{
    public:
        virtual Shape *getModel() const;
        virtual glm::mat4 getTransform() const;
        virtual Material *getMaterial() const;
        void init(physx::PxVec3 location, int id);
        void draw(MatrixStack &M);
        physx::PxRigidStatic *body;
        int lightId;
        bool pressed;
};