#include "ShaderManager.h"
#include "Utils.h"
#include "Application.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

void ShaderManager::loadShaders(std::string dir) {
    vector<string> files = listDir(dir);
    map<string, ShaderPair> shaderMap;

    for (string file : files) {
        int lastIndex = file.rfind("_");
        if (lastIndex != -1) {
            string shaderName = file.substr(0, lastIndex);
            string shaderType = file.substr(lastIndex, string::npos);
            if (shaderMap.find(shaderName) == shaderMap.end()) {
                ShaderPair pair;
                pair.vert = "";
                pair.frag = "";
                pair.geom = "";
                shaderMap[shaderName] = pair;
            }
            if (shaderType == "_vert.glsl") {
                shaderMap[shaderName].vert = dir + "/" + file;
            }
            else if (shaderType == "_frag.glsl") {
                shaderMap[shaderName].frag = dir + "/" + file;
            } 
            else if (shaderType == "_geom.glsl") {
                shaderMap[shaderName].geom = dir + "/" + file;
            }
        }
    }

    for (pair<string, ShaderPair> shaderPair : shaderMap) {
        if (!shaderPair.second.vert.empty() && !shaderPair.second.frag.empty() && shaderPair.second.geom.empty()) {
            Program shader;
            shader.setShaderNames(shaderPair.second.vert, shaderPair.second.frag);
            if (shader.init()) {
                shaders[shaderPair.first] = shader;
                cout << "Loaded shader: " << shaderPair.first << endl;
            }
        } 
        else if (!shaderPair.second.vert.empty() && !shaderPair.second.frag.empty() && !shaderPair.second.geom.empty()) {
            GeometryProgram shader;
            shader.setShaderNames(shaderPair.second.vert, shaderPair.second.frag, shaderPair.second.geom);
            if (shader.init()) {
                shaders[shaderPair.first] = shader;
                cout << "Loaded shader: " << shaderPair.first << endl;
            }
        }
    }
}

void ShaderManager::bind(std::string shaderName) {
    if (shaders.find(shaderName) != shaders.end()) {
        if (active != &shaders[shaderName]) {
            active = &shaders[shaderName];
            app.materialManager.active = nullptr;
            active->bind();
        }
    }
    else {
        cout << "Shader not found: " << shaderName << endl;
    }
}

void ShaderManager::unbind() {
    active->unbind();
    active = NULL;
}

Program *ShaderManager::getActive() {
    return active;
}

GLint ShaderManager::getAttribute(const std::string &name) const
{
	return active->getAttribute(name);
}

GLint ShaderManager::getUniform(const std::string &name) const
{
	return active->getUniform(name);
}

GLuint ShaderManager::getPid(std::string name)
{
    if (shaders.find(name) != shaders.end()) {
        Program *found = &shaders[name];
        return found->pid;
    }
}