#include "MaterialManager.h"
#include "Material.h"
#include "Application.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;
using namespace glm;

void MaterialManager::loadMaterials() {
    materials["marble"] = Material(
        "marble",
        /* spec */  vec3(0.297254, 0.30829, 0.306678),
        /* dif */   vec3(0.396, 0.74151, 0.69102),
        /* amb */   vec3(0.01, 0.018725, 0.01745),
        /* shine */ 12.8
    );

    materials["lightswitch"] = Material(
        "marble",
        /* spec */  vec3(0.74151, 0.74151, 0.0),
        /* dif */   vec3(0.74151, 0.74151, 0.0),
        /* amb */   vec3(0.01, 0.018725, 0.01745),
        /* shine */ 12.8
    );

    materials["concrete"] = Material(
        "concrete",
        /* spec */  vec3(0.508273, 0.508273, 0.508273),
        /* dif */   vec3(0.50754, 0.50754, 0.50754),
        /* amb */   vec3(0.019225, 0.019225, 0.019225),
        /* shine */ 51.2
    );

    materials["buttonUp"] = Material(
        "marble",
        /* spec */  vec3(0.727811, 0.626959, 0.626959),
        /* dif */   vec3(0.61424, 0.04136, 0.04136),
        /* amb */   vec3(0.01745, 0.01175, 0.01175),
        /* shine */ 76.2
    );

    materials["buttonDown"] = Material(
        "marble",
        /* spec */  vec3(0.633, 0.727811, 0.633),
        /* dif */   vec3(0.07568, 0.61424, 0.07568),
        /* amb */   vec3(0.0215, 0.01745, 0.0215),
        /* shine */ 76.2
    );

    materials["player"] = Material(
        "connor",
        /* spec */  vec3(0.1, 0.1, 0.1),
        /* dif */   vec3(1, 1, 1),
        /* amb */   vec3(0.01, 0.01, 0.01),
        /* shine */ 128
    );
}

void MaterialManager::bind(std::string materialName) {
    if (materials.find(materialName) != materials.end()) {
        if (active != &materials[materialName]) {
            active = &materials[materialName];
            active->bind();
        }
    }
    else {
        cout << "Material not found: " << materialName << endl;
    }
}
