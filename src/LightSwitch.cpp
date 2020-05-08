#include "Application.h"
#include "LightSwitch.h"
#include "Utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace physx;

void LightSwitch::init(physx::PxVec3 location, int id) {
    PxShape *shape = app.physics.getPhysics()->createShape(PxBoxGeometry(1, 1, 1), *app.physics.defaultMaterial);
    body = app.physics.getPhysics()->createRigidStatic(PxTransform(location));
    body->attachShape(*shape);
    app.physics.getScene()->addActor(*body);
    shape->release();

    body->userData = this;
    lightId = id;
}

void LightSwitch::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = body->getGlobalPose();
    M.translate(glm::vec3(t.p.x, t.p.y, t.p.z));
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    if (!app.renderingCubemap) {
        app.materialManager.bind("lightswitch");
    }
    app.modelManager.draw("cube");
    M.popMatrix();
}

Shape *LightSwitch::getModel() const {
    return app.modelManager.get("cube");
}

glm::mat4 LightSwitch::getTransform() const {
    PxTransform t = body->getGlobalPose();
    return translate(glm::mat4(1), px2glm(t.p));
}

Material *LightSwitch::getMaterial() const {
    return app.materialManager.get("lightswitch");
}