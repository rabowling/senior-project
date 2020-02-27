#pragma once

#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include "Texture.h"

class TextureManager
{
public:
    void loadTextures(std::string dir, bool useGl = true);
    void bind(std::string textureName, std::string uniform);
    void unbind();
    Texture *getActive();

private:
    std::unordered_map<std::string, Texture> textures;
    Texture *active = NULL;
};