#include "GameObject.h"
#include "Application.h"

using namespace glm;

#define EPSILON 0.00001

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

// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool intersectTriangle(const glm::vec3 verts[3], const glm::vec3 &orig, const glm::vec3 &dir, float &u, float &v, float &d) {
    vec3 edge1 = verts[1] - verts[0];
    vec3 edge2 = verts[2] - verts[0];
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);

    if (det < EPSILON) {
        return false;
    }

    vec3 tvec = orig - verts[0];
    u = dot(tvec, pvec);
    if (u < 0 || u > det) {
        return false;
    }

    vec3 qvec = cross(tvec, edge1);
    v = dot(dir, qvec);
    if (v < 0 || u + v > det) {
        return false;
    }

    d = dot(edge2, qvec);
    float inv_det = 1 / det;
    d *= inv_det;
    u *= inv_det;
    v *= inv_det;

    return d > -EPSILON;
}

bool GameObject::intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &u, float &v, float &d, unsigned int &faceIndex) const {
    Shape *shape = getModel();
    d = INFINITY;
    for (int fIdx = 0; fIdx < shape->eleBuf.size() / 3; fIdx++) {
        glm::vec3 verts[3] = {
            *(glm::vec3 *) &posBufCache[shape->eleBuf[fIdx*3]],
            *(glm::vec3 *) &posBufCache[shape->eleBuf[fIdx*3+1]],
            *(glm::vec3 *) &posBufCache[shape->eleBuf[fIdx*3+2]]
        };
        float tmpU, tmpV, tmpD;
        if (intersectTriangle(verts, orig, dir, tmpU, tmpV, tmpD) && tmpD < d) {
            d = tmpD;
            u = tmpU;
            v = tmpV;
            faceIndex = fIdx;
        }
    }
    return d != INFINITY;
}
