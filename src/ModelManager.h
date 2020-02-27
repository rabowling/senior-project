#pragma once

#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include "Shape.h"

class ModelManager
{
public:
    void loadModels(std::string dir, bool useGl = true);
    void draw(std::string modelName);

private:
    std::unordered_map<std::string, Shape> models;
};