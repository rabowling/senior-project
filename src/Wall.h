#pragma once

#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"
#include "GameObject.h"
#include "Material.h"

class Wall : public GameObject {
    public:
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation);
        void draw(MatrixStack &M);
        virtual Shape *getModel() const;
        virtual glm::mat4 getTransform() const;
        virtual Material *getMaterial() const;
    
        physx::PxVec3 size;
    private:
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
};