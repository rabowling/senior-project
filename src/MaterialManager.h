#pragma once

#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include "Material.h"

class MaterialManager
{
public:
    void loadMaterials();
    void bind(std::string materialName);
    Material *get(std::string materialName) { return &materials[materialName]; }
    Material *active = nullptr; // set by ShaderManager

private:
    std::unordered_map<std::string, Material> materials;
};