#include "Door.h"

#include "Application.h"
#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

using namespace physx;
using namespace glm;

void Door::init(PxVec3 position, PxVec3 size, PxQuat orientation) {
    this->size = size;
    startPos = position;
    endPos = startPos;
    endPos.y -= std::max(size.x, std::max(size.y, size.z)) * 2 + 0.1;
    pShape = app.physics.getPhysics()->createShape(PxBoxGeometry(size), *app.physics.defaultMaterial);
    gWall = app.physics.getPhysics()->createRigidStatic(PxTransform(position, orientation));
    gWall->attachShape(*pShape);
    gWall->userData = this;
    app.physics.getScene()->addActor(*gWall);
    pShape->release();
}

void Door::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = gWall->getGlobalPose();
    M.translate(vec3(t.p.x, t.p.y, t.p.z));
    M.rotate(quat(t.q.w, t.q.x, t.q.y, t.q.z));
    M.scale(vec3(size.x, size.y, size.z));
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(getTransform()));
    if (!app.renderingCubemap) {
        glUniform3f(app.shaderManager.getUniform("scale"), size.x, size.y, size.z);
        app.materialManager.bind("buttonDown");
    }
    app.modelManager.draw("cube");
    M.popMatrix();
}

Shape *Door::getModel() const {
    return app.modelManager.get("cube");
}

glm::mat4 Door::getTransform() const {
    PxTransform t = gWall->getGlobalPose();
    return scale(translate(mat4(1), px2glm(t.p)) * mat4_cast(px2glm(t.q)), px2glm(size));
}

Material *Door::getMaterial() const {
    return app.materialManager.get("buttonDown");
}

void Door::linkButton(Button *button) {
    button->linkedDoor = this;
}

void Door::open() {
    opening = true;
}

void Door::update(float dt) {
    if (opening) {
        float prevT = t;
        if (prevT < 1) {
            t += dt;
            PxTransform transform = gWall->getGlobalPose();
            transform.p = startPos + (endPos - startPos) * std::min(1.0f, t);
            gWall->setGlobalPose(transform);
        }
    }
}