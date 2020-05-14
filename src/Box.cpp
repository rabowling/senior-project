#include "Box.h"
#include "Application.h"
#include "Utils.h"
#include <glm/glm.hpp>
#include <iostream>

using namespace physx;
using namespace glm;

void Box::init(physx::PxVec3 location, physx::PxVec3 scale, physx::PxQuat rotation) {
    PxShape *shape = app.physics.getPhysics()->createShape(PxBoxGeometry(scale), *app.physics.defaultMaterial);
    body = app.physics.getPhysics()->createRigidDynamic(PxTransform(location, rotation));
    body->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    app.physics.getScene()->addActor(*body);
    shape->release();

    body->userData = this;
    this->scale = px2glm(scale);
    startPos = location;
    startRot = rotation;
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

    for (Portal *portal : touchingPortals) {
        MatrixStack camTransform;
        camTransform.translate(portal->linkedPortal->position);
        camTransform.rotate(M_PI, portal->linkedPortal->getUp());
        camTransform.rotate(portal->linkedPortal->orientation);
        camTransform.rotate(inverse(portal->orientation));
        camTransform.translate(-portal->position);
        glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(camTransform.topMatrix() * M.topMatrix()));
        app.modelManager.draw("cube");
    }

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

void Box::update(float dt) {
    prevTouchingPortals = touchingPortals;
    touchingPortals.clear();
    for (Portal &portal : app.portals) {
        if (portal.pointInBounds(px2glm(body->getGlobalPose().p))) {
            touchingPortals.push_back(&portal);
        }
    }

    for (Portal *portal : prevTouchingPortals) {
        if (!portal->facing(px2glm(body->getGlobalPose().p))) {
            MatrixStack camTransform;
            camTransform.translate(portal->linkedPortal->position);
            camTransform.rotate(M_PI, portal->linkedPortal->getUp());
            camTransform.rotate(portal->linkedPortal->orientation);
            camTransform.rotate(inverse(portal->orientation));
            camTransform.translate(-portal->position);

            vec3 newPos = vec3(camTransform.topMatrix() * vec4(px2glm(body->getGlobalPose().p), 1));
            body->setGlobalPose(PxTransform(glm2px(newPos), body->getGlobalPose().q));

            vec3 newVel = vec3(camTransform.topMatrix() * vec4(px2glm(body->getLinearVelocity()), 0));
            body->setLinearVelocity(glm2px(newVel));

            app.player.holdTransform = camTransform.topMatrix() * app.player.holdTransform;

            prevTouchingPortals = touchingPortals;
            touchingPortals.clear();
            for (Portal &portal : app.portals) {
                if (portal.pointInBounds(px2glm(body->getGlobalPose().p))) {
                    touchingPortals.push_back(&portal);
                }
            }
            break;
        }
    }

    if (body->getGlobalPose().p.y < -50) {
        respawn();
    }
}

void Box::respawn() {
    body->setLinearVelocity(PxVec3(0,0,0));
    body->setGlobalPose(PxTransform(startPos, startRot));
}

void Box::onContactModify(const physx::PxRigidActor *actor, physx::PxContactSet &contacts) {
    for (int i = 0; i < contacts.size(); i++) {
        vec3 contactPoint = px2glm(contacts.getPoint(i));
        for (Portal *portal : touchingPortals) {
            if (portal->pointInSideBounds(contactPoint) && (actor == portal->surface || !portal->facing(contactPoint))) {
                contacts.ignore(i);
                break;
            }
        }
    }
}
