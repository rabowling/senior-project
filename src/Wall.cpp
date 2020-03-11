#include "Wall.h"
#include "Application.h"
#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace physx;
using namespace glm;

void Wall::init(PxVec3 position, PxVec3 size, PxQuat orientation) {
    this->size = size;

    pShape = app.physics.getPhysics()->createShape(PxBoxGeometry(size), *app.physics.defaultMaterial);
    gWall = app.physics.getPhysics()->createRigidStatic(PxTransform(position, orientation));
    gWall->attachShape(*pShape);
    app.physics.getScene()->addActor(*gWall);
    pShape->release();
}

void Wall::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = gWall->getGlobalPose();
    M.translate(vec3(t.p.x, t.p.y, t.p.z));
    M.rotate(quat(t.q.w, t.q.x, t.q.y, t.q.z));
    M.scale(vec3(size.x, size.y, size.z));
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(getTransform()));
    if (!app.renderingCubemap) {
        glUniform3f(app.shaderManager.getUniform("scale"), size.x, size.y, size.z);
        app.materialManager.bind("concrete");
    }
    app.modelManager.draw("cube");
    M.popMatrix();
}

Shape *Wall::getModel() const {
    return app.modelManager.get("cube");
}

glm::mat4 Wall::getTransform() const {
    PxTransform t = gWall->getGlobalPose();
    return scale(translate(mat4(1), px2glm(t.p)) * mat4_cast(px2glm(t.q)), px2glm(size));
}

Material *Wall::getMaterial() const {
    return app.materialManager.get("concrete");
}
