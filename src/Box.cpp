#include "Box.h"
#include "Application.h"
#include "Utils.h"
#include <glm/glm.hpp>

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
    if (!app.renderingCubemap) {
        app.materialManager.bind("marble");
    }
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw("cube");
    M.popMatrix();
}

Shape *Box::getModel() const {
    return app.modelManager.get("cube");
}

glm::mat4 Box::getTransform() const {
    PxTransform t = body->getGlobalPose();
    return glm::scale(glm::translate(glm::mat4(1), px2glm(t.p)) * glm::mat4_cast(px2glm(t.q)), scale);
}

Material *Box::getMaterial() const {
    return app.materialManager.get("marble");
}
