#include "Material.h"
#include "Application.h"

using namespace glm;

Material::Material() {

}

Material::Material(std::string texture, glm::vec3 spec, glm::vec3 dif, glm::vec3 amb, float shine) :
    texture(texture), spec(spec), dif(dif), amb(amb), shine(shine)
{

}

void Material::bind() {
    glUniform3fv(app.shaderManager.getUniform("MatAmb"), 1, glm::value_ptr(amb));
    glUniform3fv(app.shaderManager.getUniform("MatDif"), 1, glm::value_ptr(dif));
    glUniform3fv(app.shaderManager.getUniform("MatSpec"), 1, glm::value_ptr(spec));
    glUniform1f(app.shaderManager.getUniform("Shine"), shine);
    app.textureManager.bind(texture, "Texture0");
}

Texture *Material::getTexture() {
    return app.textureManager.get(texture);
}