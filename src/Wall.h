#pragma once

#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"
#include "GameObject.h"

class Wall : public GameObject {
    public:
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation);
        void draw(MatrixStack &M, const bool isCubemap);
        virtual Shape *getModel() const;
        virtual glm::mat4 getTransform() const;
    
        physx::PxVec3 size;
    private:
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
};