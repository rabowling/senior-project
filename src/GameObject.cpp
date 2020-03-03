#include "GameObject.h"
#include "Application.h"

using namespace glm;

void GameObject::cacheGeometry() {
    mat4 transform = getTransform();
    Shape *shape = getModel();
    if (posBufCache.size() != shape->posBuf.size()) {
        posBufCache.resize(shape->posBuf.size());
    }

    for (int fIdx = 0; fIdx < shape->eleBuf.size() / 3; fIdx++) {
        vec3 v[3];
        for (int vNum = 0; vNum < 3; vNum++) {
            unsigned int vIdx = shape->eleBuf[fIdx*3+vNum];
            for (int i = 0; i < 3; i++) {
                v[vNum][i] = shape->posBuf[vIdx*3+i];
            }
            v[vNum] = vec3(transform * vec4(v[vNum], 1));
            for (int i = 0; i < 3; i++) {
                posBufCache[vIdx*3+i] = v[vNum][i];
            }
        }
    }

    cacheFrame = app.stepCount;
}