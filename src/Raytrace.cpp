#include "Raytrace.h"
#include "Shape.h"
#include <vector>
#include "Application.h"
#include "GameObject.h"
#include <list>
#include <fstream>
#include <iostream>

#define EPSILON 0.00001

using namespace glm;
using namespace std;

bool intersectRayTriangle(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 &v0, const glm::vec3 &v1,
    const glm::vec3 &v2, float &d, float &u, float &v)
{
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);

    if (det < EPSILON) {
        return false;
    }

    vec3 tvec = orig - v0;
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

    return d > 0;
}

bool compare(const RayHit &r1, const RayHit &r2) {
    if (abs(r1.d - r2.d) < 0.001) {
        if (dynamic_cast<Portal *>(r1.obj)) {
            return true;
        }
        else if (dynamic_cast<Portal *>(r2.obj)) {
            return false;
        }
    }
    return r1.d < r2.d;
}

bool intersectRayShape(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<float> &posBuf, const std::vector<unsigned int> eleBuf,
    RayHit &closestHit)
{
    bool success = false;
    for (int fIdx = 0; fIdx < eleBuf.size() / 3; fIdx++) {
        vec3 v[3];
        for (int vNum = 0; vNum < 3; vNum++) {
            unsigned int vIdx = eleBuf[fIdx*3+vNum];
            for (int i = 0; i < 3; i++) {
                v[vNum][i] = posBuf[vIdx*3+i];
            }
        }

        RayHit hit;
        if (intersectRayTriangle(orig, dir, v[0], v[1], v[2], hit.d, hit.u, hit.v)) {
            if (!success || hit.d < closestHit.d) {
                success = true;
                hit.faceIndex = fIdx;
                closestHit = hit;
            }
        }
    }
    return success;
}

bool traceGameObject(const glm::vec3 &orig, const glm::vec3 &dir, const GameObject &obj, RayHit &hit) {
    return intersectRayShape(orig, dir, obj.posBufCache, obj.getModel()->eleBuf, hit);
}

glm::vec3 traceScene(const glm::vec3 &orig, const glm::vec3 &dir) {
    RayHit closestHit;
    bool success = false;
    
    for (GameObject *obj : app.gameObjects) {
        RayHit hit;
        if (traceGameObject(orig, dir, *obj, hit)) {
            hit.obj = obj;
            if (!success || compare(hit, closestHit)) {
                success = true;
                closestHit = hit;
            }
        }
    }
    
    vec3 color(0);
    if (success) {
        color = vec3(closestHit.d / 50);
        if (dynamic_cast<Button *>(closestHit.obj)) {
            color.g = color.b = 0;
        }
        else if (dynamic_cast<Wall *>(closestHit.obj)) {
            color.r = color.b = 0;
        }
        else if (dynamic_cast<Box *>(closestHit.obj)) {
            color.r = color.g = 0;
        }
        else if (dynamic_cast<Portal *>(closestHit.obj)) {
            color.r = 0;
        }
    }
    return color;
}

void renderRT(int width, int height, const std::string &filename) {
    vec3 *pixels = new vec3[width * height];
    float fov = 45;
    float invHeight = 1.0f / height;
    float invWidth = 1.0f / width;
    float aspect = width * invHeight;
    float angle = tan(M_PI * 0.5 * fov / 180);
    mat4 view = mat4_cast(quatLookAt(app.player.camera.lookAtPoint - app.player.camera.eye, app.player.camera.upVec));

    for (GameObject *obj : app.gameObjects) {
        obj->cacheGeometry();
    }

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspect;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            vec3 dir = normalize(vec3(view * vec4(xx, yy, -1, 0)));
            vec3 orig = app.player.camera.eye;
            pixels[y*width+x] = traceScene(orig, dir);
        }
    }

    std::ofstream ofs(filename, std::ios::out | std::ios::binary); 
    ofs << "P6\n" << width << " " << height << "\n255\n"; 
    for (unsigned i = 0; i < width * height; ++i) { 
        ofs << (unsigned char)(std::min(float(1), pixels[i].r) * 255) << 
               (unsigned char)(std::min(float(1), pixels[i].g) * 255) << 
               (unsigned char)(std::min(float(1), pixels[i].b) * 255); 
    }
    ofs.close(); 

    delete[] pixels;
}
