#include "TextureManager.h"
#include "Utils.h"
#include "Application.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

void TextureManager::loadTextures(std::string dir) {
    vector<string> files = listDir(dir);

    for (string file : files) {
        int lastIndex = file.rfind(".");
        if (lastIndex != -1) {
            string textureName = file.substr(0, lastIndex);
            Texture texture;
            texture.setFilename(dir + "/" + file);
            texture.init();
            texture.setUnit(1);
            texture.setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            textures[textureName] = texture;
            cout << "Loaded texture: " << textureName << endl;
        }
    }
}

void TextureManager::bind(std::string textureName, std::string uniform) {
    if (textures.find(textureName) != textures.end()) {
        if (active != &textures[textureName]) {
            active = &textures[textureName];
            active->bind(app.shaderManager.getUniform(uniform));
        }
    }
    else {
        cout << "Texture not found: " << textureName << endl;
    }
}

void TextureManager::unbind() {
    active->unbind();
    active = NULL;
}

Texture *TextureManager::getActive() {
    return active;
}
