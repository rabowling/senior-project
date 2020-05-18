#include "Button.h"

#include "Application.h"
#include <string>
#include "Utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Door.h"

using namespace physx;

void Button::onContact(PxActor *other) {
    pressed = true;
    if (linkedDoor != nullptr) {
        linkedDoor->open();
    }
}

void Button::init(physx::PxVec3 location) {
    PxShape *shape = app.physics.getPhysics()->createShape(PxBoxGeometry(1.5, 0.2, 1.5), *app.physics.defaultMaterial);
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
    if (!app.renderingCubemap) {
        app.materialManager.bind(pressed ? "buttonDown" : "buttonUp");
    }
    app.modelManager.draw("button");
    M.popMatrix();
}

Shape *Button::getModel() const {
    return app.modelManager.get("button");
}

glm::mat4 Button::getTransform() const {
    PxTransform t = body->getGlobalPose();
    return glm::translate(glm::mat4(1), px2glm(t.p));
}

Material *Button::getMaterial() const {
    return app.materialManager.get(pressed ? "buttonDown" : "buttonUp");
}
