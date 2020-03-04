#pragma once

#include <string>
#include <glm/glm.hpp>
#include "Texture.h"

class Material
{
public:
    Material();
    Material(std::string texture, glm::vec3 spec, glm::vec3 dif, glm::vec3 amb, float shine);
    std::string texture;
    glm::vec3 spec;
    glm::vec3 dif;
    glm::vec3 amb;
    float shine;

    void bind();
    Texture *getTexture();
};