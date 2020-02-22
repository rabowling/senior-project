#include "Portal.h"
#include "Application.h"

#include <glm/gtc/quaternion.hpp>

using namespace glm;

void Portal::setPosition(glm::vec3 position, glm::quat orientation) {
    glm::vec3 lookDir = vec3(mat4_cast(orientation) * vec4(localForward, 0));
    this->position = position;
    this->orientation = orientation;
    camera.init(position, lookDir, getUp());
}

void Portal::draw(MatrixStack &M) {
    M.pushMatrix();
    M.translate(position);
    M.rotate(orientation);
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw(model);
    M.popMatrix();
}

void Portal::drawOutline(MatrixStack &M) {
    M.pushMatrix();
    M.translate(position);
    M.rotate(orientation);
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    app.modelManager.draw("portal_outline");
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
    return vec3(mat4_cast(orientation) * vec4(localUp, 0));
}

vec3 Portal::getForward() {
    return vec3(mat4_cast(orientation) * vec4(localForward, 0));
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