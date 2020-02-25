#include "Box.h"
#include "Application.h"
#include "Utils.h"

using namespace physx;

void Box::init(physx::PxVec3 location, physx::PxVec3 scale, physx::PxQuat rotation) {
    PxShape *shape = app.physics.getPhysics()->createShape(PxBoxGeometry(scale), *app.physics.defaultMaterial);
    body = app.physics.getPhysics()->createRigidDynamic(PxTransform(location, rotation));
    body->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    app.physics.getScene()->addActor(*body);
    shape->release();

    body->userData = this;
    this->scale = px2glm(scale);
}

void Box::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = body->getGlobalPose();
    M.translate(glm::vec3(t.p.x, t.p.y, t.p.z));
    M.rotate(glm::quat(t.q.w, t.q.x, t.q.y, t.q.z));
    M.scale(scale);
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw("cube");
    M.popMatrix();
}