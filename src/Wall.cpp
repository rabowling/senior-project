#include "Wall.h"

using namespace physx;
using namespace glm;

Wall::Wall() {
}

void Wall::init(PxVec3 position, PxVec3 size, PxQuat orientation, Physics physics, std::shared_ptr<Shape> shape) {
    this->size = size;
    this->shape = shape;

    PxMaterial *material = physics.getPhysics()->createMaterial(0.3f, 0.3f, 0.3f);
    pShape = physics.getPhysics()->createShape(PxBoxGeometry(size), *material);
    gWall = physics.getPhysics()->createRigidStatic(PxTransform(position, orientation));
    gWall->attachShape(*pShape);
    physics.getScene()->addActor(*gWall);
    pShape->release();
}

void Wall::draw(ShaderManager manager, std::shared_ptr<MatrixStack> M) {
    M->pushMatrix();
    PxTransform t = gWall->getGlobalPose();
    M->translate(vec3(t.p.x, t.p.y, t.p.z));
    M->rotate(quat(t.q.w, t.q.x, t.q.y, t.q.z));
    M->scale(vec3(size.x, size.y, size.z));
    glUniformMatrix4fv(manager.getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    glUniform3f(manager.getUniform("scale"), size.x, size.y, size.z);
    shape->draw(manager.getActive());
    M->popMatrix();
}