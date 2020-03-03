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

bool traceScene(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &closestHit) {
    bool success = false;
    
    for (GameObject *obj : app.gameObjects) {
        RayHit hit;
        if (intersectRayShape(orig, dir, obj->posBufCache, obj->getModel()->eleBuf, hit)) {
            hit.obj = obj;
            if (!success || compare(hit, closestHit)) {
                success = true;
                closestHit = hit;
            }
        }
    }

    return success;
}

glm::vec3 traceColor(const glm::vec3 &orig, const glm::vec3 &dir) {
    RayHit hit;
    if (!traceScene(orig, dir, hit)) {
        return vec3(0, 0, 0);
    }

    vec3 vert[3];
    vec2 vt[3];
    vec3 vn[3];
    Shape *model = hit.obj->getModel();
    for (int vNum = 0; vNum < 3; vNum++) {
        unsigned int vIdx = model->eleBuf[hit.faceIndex*3+vNum];
        for (int i = 0; i < 3; i++) {
            vert[vNum][i] = hit.obj->posBufCache[vIdx*3+i];
            vn[vNum][i] = model->norBuf[vIdx*3+i];
        }

        for (int i = 0; i < 2; i++) {
            vt[vNum][i] = model->texBuf[vIdx*2+i];
        }
    }

    vec2 uv = hit.u * vt[1] + hit.v * vt[2] + (1 - hit.u - hit.v) * vt[0];

    Texture *texture;
    if (dynamic_cast<Wall *>(hit.obj)) {
        texture = app.textureManager.get("concrete");
        Wall *wall = static_cast<Wall *>(hit.obj);
        if (dot(vn[0], vec3(1, 0, 0)) != 0) {
            uv.x *= wall->size.y;
            uv.y *= wall->size.z;
        } else if (dot(vn[0], vec3(0, 1, 0)) != 0) {
            uv.x *= wall->size.x;
            uv.y *= wall->size.z;
        } else if (dot(vn[0], vec3(0, 0, 1)) != 0) {
            uv.x *= wall->size.y;
            uv.y *= wall->size.x;
        }
    }
    else {
        texture = app.textureManager.get("marble");
    }

    if (uv.x > 1.0f) {
        uv.x = fmod(uv.x, 1.0f);
    }
    if (uv.y > 1.0f) {
        uv.y = fmod(uv.y, 1.0f);
    }

    uv.x *= texture->width;
    uv.y *= texture->height;

    // blerp
    ivec2 center = round(uv);
    vec2 delta = (vec2) center - uv + 0.5f;
    vec3 texColor(0);
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            int idxX = center.x + x - 1;
            int idxY = center.y + y - 1;

            if (idxX == texture->width) {
                idxX = 0;
            }
            else if (idxX == -1) {
                idxX = texture->width - 1;
            }
            if (idxY == texture->height) {
                idxY = 0;
            }
            else if (idxY == -1) {
                idxY = texture->height - 1;
            }

            int idx = idxY * texture->width + idxX;
            vec3 sample;
            for (int i = 0; i < 3; i++) {
                sample[i] = texture->data[idx*3+i];
            }
            texColor += sample * (x == 0 ? delta.x : 1 - delta.x) * (y == 0 ? delta.y : 1 - delta.y);
        }
    }

    // Blinn-Phong shading
    vec3 hitPos = hit.u * vert[1] + hit.v * vert[2] + (1 - hit.u - hit.v) * vert[0];
    vec3 lightPos(30, 8, 30);
    vec3 normal = normalize(cross(vert[1] - vert[0], vert[2] - vert[0]));
    vec3 lightDir = normalize(lightPos - hitPos);
    vec3 diffuse = texColor * std::max(0.f, dot(normal, lightDir));
    vec3 viewDir = normalize(app.player.camera.eye - hitPos);
    vec3 H = normalize((lightDir + viewDir) / 2.f);
    vec3 specular = vec3(1) * glm::pow(std::max(0.f, dot(H, normal)), 12.8f);

    // Shadow rays
    RayHit shadowRayHit;
    float shadow = 1;
    if (traceScene(hitPos, normalize(lightPos - hitPos), shadowRayHit)) {
        if (shadowRayHit.d < distance(lightPos, hitPos)) {
            shadow = 0;
        }
    }

    return (diffuse + specular) * shadow;
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
            pixels[y*width+x] = traceColor(orig, dir);
        }
    }

    std::ofstream ofs(filename, std::ios::out | std::ios::binary); 
    ofs << "P6\n" << width << " " << height << "\n255\n"; 
    for (unsigned i = 0; i < width * height; ++i) { 
        ofs << (unsigned char)(std::min(255, (int) round(pixels[i].r))) << 
               (unsigned char)(std::min(255, (int) round(pixels[i].g))) << 
               (unsigned char)(std::min(255, (int) round(pixels[i].b))); 
    }
    ofs.close(); 

    delete[] pixels;
}
