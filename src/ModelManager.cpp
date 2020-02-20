#include "ModelManager.h"
#include "Utils.h"
#include "Application.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

void ModelManager::loadModels(std::string dir) {
    vector<string> files = listDir(dir);

    for (string file : files) {
        int lastIndex = file.rfind(".");
        if (lastIndex != -1) {
            string modelName = file.substr(0, lastIndex);
            Shape shape;
            shape.loadMesh(dir + "/" + file);
            shape.init();
            models[modelName] = shape;
            cout << "Loaded model: " << modelName << endl;
        }
    }
}

void ModelManager::draw(std::string modelName) {
    if (models.find(modelName) != models.end()) {
        models[modelName].draw(app.shaderManager.getActive());
    }
    else {
        cout << "Model not found: " << modelName << endl;
    }
}