#pragma once

#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"

class Wall {
    public:
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation);
        void draw(MatrixStack &M);
    
    private:
        physx::PxVec3 size;
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
};