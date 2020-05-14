#include "MiscItem.h"
#include "Application.h"
#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace physx;
using namespace glm;
using namespace std;

void MiscItem::init(PxVec3 position, PxVec3 size, PxQuat orientation, string s, string mat) {
    this->size = size;

    pShape = app.physics.getPhysics()->createShape(PxBoxGeometry(size), *app.physics.defaultMaterial);
    gWall = app.physics.getPhysics()->createRigidStatic(PxTransform(position, orientation));
    gWall->attachShape(*pShape);
    app.physics.getScene()->addActor(*gWall);
    pShape->release();

    shape = s;
    material = mat;
}

void MiscItem::draw(MatrixStack &M) {
    M.pushMatrix();
    PxTransform t = gWall->getGlobalPose();
    M.translate(vec3(t.p.x, t.p.y, t.p.z));
    M.rotate(quat(t.q.w, t.q.x, t.q.y, t.q.z));
    M.scale(vec3(size.x, size.y, size.z));
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(getTransform()));
    if (!app.renderingCubemap) {
        app.materialManager.bind(material);
    }
    app.modelManager.draw(shape);
    M.popMatrix();
}

Shape *MiscItem::getModel() const {
    return app.modelManager.get(shape);
}

glm::mat4 MiscItem::getTransform() const {
    PxTransform t = gWall->getGlobalPose();
    return scale(translate(mat4(1), px2glm(t.p)) * mat4_cast(px2glm(t.q)), px2glm(size));
}

Material *MiscItem::getMaterial() const {
    return app.materialManager.get("concrete");
}
