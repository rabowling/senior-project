#pragma once

#include <unordered_map>
#include <string>
#include "Program.h"
#include "GeometryProgram.h"

class ShaderManager
{
public:
    void loadShaders(std::string dir);
    void bind(std::string shaderName);
    void unbind();
    GLint getAttribute(const std::string &name) const;
	GLint getUniform(const std::string &name) const;
    GLuint getPid(std::string name);
    Program *getActive();

private:
    void addUniformsAndAttributes();
    std::unordered_map<std::string, Program> shaders;
    Program *active = NULL;
};

struct ShaderPair
{
    std::string vert;
    std::string frag;
    std::string geom;
};