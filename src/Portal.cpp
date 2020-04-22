#include "Portal.h"
#include "Application.h"
#include "Utils.h"
#include "PortalOutline.h"
#include <iostream>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

Portal::Portal(glm::vec3 position, glm::vec3 scale, glm::quat orientation, std::string model) :
    scale(scale), model(model)
{
    setPosition(position, orientation);
}

void Portal::setOutline(PortalOutline *outline) {
    this->outline = outline;
    hasOutline = true;
    outline->parent = this;
}

void Portal::setPosition(glm::vec3 position, glm::quat orientation) {
    glm::vec3 lookDir = vec3(mat4_cast(orientation) * vec4(localForward, 0));
    this->position = position;
    this->orientation = orientation;
    camera.init(position, lookDir, getUp());
    isForwardCached = false;
    isUpCached = false;

    // portal plane
    vec3 forward = getForward();
    vec3 planePos = position;
    bounds[0] = vec4(forward, -dot(planePos, forward));

    // up plane
    vec3 up = getUp();
    planePos = position + up * 2.f;
    bounds[1] = vec4(-up, -dot(planePos, -up));

    // down plane
    planePos = position - up * 2.f;
    bounds[2] = vec4(up, -dot(planePos, up));

    // right plane
    vec3 right = cross(up, forward);
    planePos = position + right * 2.f;
    bounds[3] = vec4(-right, -dot(planePos, -right));

    // left plane
    planePos = position - right * 2.f;
    bounds[4] = vec4(right, -dot(planePos, right));
}

void Portal::draw(MatrixStack &M) {
    M.pushMatrix();
    M.translate(position);
    M.rotate(orientation);
    M.scale(scale);
    if (!app.renderingCubemap) {
        glUniform3fv(app.shaderManager.getUniform("outlinecolor"), 1, value_ptr(hasOutline ? outline->color : vec3(1)));
    }
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw(model);
    M.popMatrix();
}

void Portal::updateCamera(const Camera &playerCamera) {
    MatrixStack camTransform;
    camTransform.translate(position);
    camTransform.rotate(M_PI, getUp());
    camTransform.rotate(orientation);
    camTransform.rotate(inverse(linkedPortal->orientation));
    camTransform.translate(-linkedPortal->position);

    camera.eye = vec3(camTransform.topMatrix() * vec4(playerCamera.eye, 1));
    camera.lookAtPoint = vec3(camTransform.topMatrix() * vec4(playerCamera.lookAtPoint, 1));
    camera.upVec = vec3(camTransform.topMatrix() * vec4(playerCamera.upVec, 0));
}

void Portal::linkPortal(Portal *other) {
    linkedPortal = other;
    other->linkedPortal = this;
}

vec3 Portal::getUp() {
    if (!isUpCached) {
        cachedUp = vec3(mat4_cast(orientation) * vec4(localUp, 0));
        isUpCached = true;
    }
    return cachedUp;
}

vec3 Portal::getForward() {
    if (!isForwardCached) {
        cachedForward = vec3(mat4_cast(orientation) * vec4(localForward, 0));
        isForwardCached = true;
    }
    return cachedForward;
}

bool Portal::facing(const glm::vec3 &point) {
    vec3 normal = getForward();
    float D = -dot(normal, position);
    return dot(normal, point) + D > 0;
}

bool Portal::pointInBounds(const glm::vec3 &point) {
    for (const vec4 &plane : bounds) {
        if (dot(vec3(plane), point) + plane.w < 0) {
            return false;
        }
    }

    return true;
}

bool Portal::pointInSideBounds(const glm::vec3 &point) {
    for (int i = 1; i < 5; i++) {
        if (dot(vec3(bounds[i]), point) + bounds[i].w < 0) {
            return false;
        }
    }

    return true;
}

// https://aras-p.info/texts/obliqueortho.html
// Set the near plane of the projection matrix
mat4 Portal::modifyProjectionMatrix(const mat4 &P, const mat4 &V) {
    vec3 norm = vec3(V * vec4(getForward(), 0));
    vec3 point = vec3(V * vec4(position, 1));
    vec4 clipPlane = vec4(norm, -dot(norm, point));
    mat4 mat = P;

    vec4 q = inverse(mat) * vec4(
        sign(clipPlane.x),
        sign(clipPlane.y),
        1,
        1
    );
    vec4 c = clipPlane * (2 / dot(clipPlane, q));
    mat[0][2] = c.x - mat[0][3];
    mat[1][2] = c.y - mat[1][3];
    mat[2][2] = c.z - mat[2][3];
    mat[3][2] = c.w - mat[3][3];

    return mat;
}

Shape *Portal::getModel() const {
    return app.modelManager.get(model);
}

glm::mat4 Portal::getTransform() const {
    return glm::scale(translate(mat4(1), position) * mat4_cast(orientation), scale);
}

Material *Portal::getMaterial() const {
    return nullptr;
}
