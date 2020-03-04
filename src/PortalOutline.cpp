#include "PortalOutline.h"
#include "Application.h"

PortalOutline::PortalOutline(glm::vec3 color, std::string model) : color(color), model(model)
{

}


void PortalOutline::draw(MatrixStack &M) {
    M.pushMatrix();
    M.loadIdentity();
    M.translate(parent->position);
    M.rotate(parent->orientation);
    M.scale(parent->scale);
    glUniformMatrix4fv(app.shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M.topMatrix()));
    glUniform3fv(app.shaderManager.getUniform("outlinecolor"), 1, value_ptr(color));
    app.modelManager.draw(model);
    M.popMatrix();
}

Shape *PortalOutline::getModel() const {
    return app.modelManager.get(model);
}

glm::mat4 PortalOutline::getTransform() const {
    return parent->getTransform();
}

Material *PortalOutline::getMaterial() const {
    return nullptr;
}