#include "Physics.h"
#include "Shape.h"
#include "ShaderManager.h"
#include "MatrixStack.h"

class Wall {
    public:
        Wall();
        void init(physx::PxVec3 position, physx::PxVec3 size, physx::PxQuat orientation, Physics physics, std::shared_ptr<Shape> shape);
        void draw(ShaderManager manager, std::shared_ptr<MatrixStack> M);
    
    private:
        physx::PxVec3 size;
        physx::PxShape *pShape;
        physx::PxRigidStatic *gWall;
        std::shared_ptr<Shape> shape;
};