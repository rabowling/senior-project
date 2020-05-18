#pragma once

#include "Wall.h"
#include "Button.h"
#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"
#include "GameObject.h"
#include "Material.h"

class Door : public GameObject {
    public:
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation);
        void linkButton(Button *button);
        void open();
        void draw(MatrixStack &M);
        void update(float dt);
        virtual Shape *getModel() const;
        virtual glm::mat4 getTransform() const;
        virtual Material *getMaterial() const;
    
        physx::PxVec3 size;
    private:
        physx::PxVec3 startPos;
        physx::PxVec3 endPos;
        float speed = 3;
        float t = 0;
        bool opening = false;
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
};
