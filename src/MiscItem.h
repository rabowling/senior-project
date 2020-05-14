#pragma once

#include <stdlib.h>
#include <string>
#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"
#include "GameObject.h"
#include "Material.h"

class MiscItem : public GameObject {
    public:
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation, std::string s, std::string mat);
        void draw(MatrixStack &M);
        virtual Shape *getModel() const;
        virtual glm::mat4 getTransform() const;
        virtual Material *getMaterial() const;
    
        physx::PxVec3 size;
    private:
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
        std::string shape;
        std::string material;
};