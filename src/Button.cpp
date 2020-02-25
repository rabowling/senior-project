#include "Button.h"

#include "Application.h"
#include <string>

using namespace physx;

void Button::onContact(PxActor *other) {
    pressed = true;
}

void Button::init(physx::PxVec3 location) {
    PxShape *shape = app.physics.getPhysics()->createShape(PxBoxGeometry(1, 0.4, 1), *app.physics.defaultMaterial);
    body = app.physics.getPhysics()->createRigidStatic(PxTransform(location));
    body->attachShape(*shape);
    app.physics.getScene()->addActor(*body);
    shape->release();

    body->userData = this;
}

void Button::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = body->getGlobalPose();
    M.translate(glm::vec3(t.p.x, t.p.y, t.p.z));
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw("cylinder");
    M.popMatrix();
}