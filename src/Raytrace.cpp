#include "Raytrace.h"
#include "Shape.h"
#include <vector>
#include "Application.h"
#include "GameObject.h"
#include "Material.h"
#include "KDTree.h"
#include <list>
#include <fstream>
#include <iostream>
#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

using namespace glm;
using namespace std;

const int NUM_BOUNCES = 1;
const int NUM_BOUNCE_RAYS = 16;
const int LIGHT_RADIUS = 2;
const int NUM_SHADOW_SAMPLES_X = 3;
const int NUM_SHADOW_SAMPLES_Y = 3;

glm::vec3 randomDirInSphere(const glm::vec3 &normal) {
    vec3 dir = normalize(vec3(rand() % 2000 - 1000, rand() % 2000 - 1000, rand() % 2000 - 1000));
    if (dot(dir, normal) < 0) {
        dir = -dir;
    }
    return dir;
}

bool fastCheckPortal(const glm::vec3 &orig, const glm::vec3 &dir, Portal &portal) {
    float d;
    if (glm::intersectRayPlane(orig, dir, portal.position, portal.getForward(), d)) {
        vec3 hitPos = orig + dir * d;
        return portal.pointInSideBounds(hitPos);
    }
    return false;
}

bool checkShadow(const glm::vec3 pos, const glm::vec3 lightPos, const KdTreeAccel &kdtree) {
    Ray shadowRay(pos, normalize(lightPos - pos), distance(lightPos, pos));
    return kdtree.IntersectP(shadowRay);
}

bool checkShadowThroughPortal(const glm::vec3 pos, const glm::vec3 &lightPos, Portal &portal, const KdTreeAccel &kdtree, glm::vec3 &transformedLightPos) {
    if (!portal.open || !portal.linkedPortal->open || !portal.facing(pos) || !portal.linkedPortal->facing(lightPos)) {
        return true;
    }

    MatrixStack camTransform2;
    camTransform2.translate(portal.position);
    camTransform2.rotate(M_PI, portal.getUp());
    camTransform2.rotate(portal.orientation);
    camTransform2.rotate(inverse(portal.linkedPortal->orientation));
    camTransform2.translate(-portal.linkedPortal->position);

    transformedLightPos = vec3(camTransform2.topMatrix() * vec4(lightPos, 1));
    vec3 lightDir = normalize(transformedLightPos - pos);
    Ray portalRay(pos, lightDir);

    SurfaceInteraction shadowRayHit;
    if (fastCheckPortal(pos, lightDir, portal)
            && kdtree.Intersect(portalRay, shadowRayHit)
            && shadowRayHit.tri->obj == &portal) {
        vec3 (&vert2)[3] = shadowRayHit.tri->verts;
        Shape *model2 = shadowRayHit.tri->obj->getModel();

        MatrixStack camTransform;
        camTransform.translate(portal.linkedPortal->position);
        camTransform.rotate(M_PI, portal.linkedPortal->getUp());
        camTransform.rotate(portal.linkedPortal->orientation);
        camTransform.rotate(inverse(portal.orientation));
        camTransform.translate(-portal.position);

        vec3 shadowHitPos = shadowRayHit.u * vert2[1] + shadowRayHit.v * vert2[2] + (1 - shadowRayHit.u - shadowRayHit.v) * vert2[0];
        vec3 shadowOrig = vec3(camTransform.topMatrix() * vec4(shadowHitPos, 1));
        return checkShadow(shadowOrig, lightPos, kdtree);
    }
    return true;
}

glm::vec3 traceColor(const Ray &ray, const KdTreeAccel &kdtree, int bounceDepth = 0) {
    SurfaceInteraction hit;
    if (!kdtree.Intersect(ray, hit)) {
        return vec3(0, 0, 0);
    }

    vec3 (&vert)[3] = hit.tri->verts;
    vec2 vt[3];
    vec3 vn[3];
    Shape *model = hit.tri->obj->getModel();
    for (int vNum = 0; vNum < 3; vNum++) {
        unsigned int vIdx = model->eleBuf[hit.tri->faceIndex*3+vNum];
        for (int i = 0; i < 3; i++) {
            vn[vNum][i] = model->norBuf[vIdx*3+i];
        }

        for (int i = 0; i < 2; i++) {
            vt[vNum][i] = model->texBuf[vIdx*2+i];
        }
    }

    vec3 hitPos = hit.u * vert[1] + hit.v * vert[2] + (1 - hit.u - hit.v) * vert[0];
    vec3 hitNorm = normalize(cross(vert[1] - vert[0], vert[2] - vert[0]));

    Material *material = hit.tri->obj->getMaterial();
    if (material) {
        Texture *texture = material->getTexture();
        vec2 uv = hit.u * vt[1] + hit.v * vt[2] + (1 - hit.u - hit.v) * vt[0];

        // scale UV for Wall objects
        if (dynamic_cast<Wall *>(hit.tri->obj)) {
            Wall *wall = static_cast<Wall *>(hit.tri->obj);
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

        vec3 color(0);
        for (const Light &light : app.lights) {
            // Blinn-Phong shading
            vec3 ambient = material->amb * texColor * light.intensity;
            //color += ambient;

            // Shadow rays
            vector<Light> lightSamples;
            vec3 lightForward = normalize(hitPos - light.position);
            vec3 lightRight = cross(vec3(0, 1, 0), lightForward);
            vec3 lightUp = cross(lightForward, lightRight);
            vec3 directLight(0);
            for (int x = 0; x < NUM_SHADOW_SAMPLES_X; x++) {
                for (int y = 0; y < NUM_SHADOW_SAMPLES_Y; y++) {
                    float offsetX = 0;
                    float offsetY = 0;
                    if (NUM_SHADOW_SAMPLES_X * NUM_SHADOW_SAMPLES_Y > 1) {
                        offsetX = (x - NUM_SHADOW_SAMPLES_X / 2.f + 0.5f + (rand() / (float) RAND_MAX - 0.5)) / NUM_SHADOW_SAMPLES_X * LIGHT_RADIUS;
                        offsetY = (y - NUM_SHADOW_SAMPLES_Y / 2.f + 0.5f + (rand() / (float) RAND_MAX - 0.5)) / NUM_SHADOW_SAMPLES_Y * LIGHT_RADIUS;
                        //offsetX = (2 * rand() / (float) RAND_MAX - 1) * LIGHT_RADIUS;
                        //offsetX = (2 * rand() / (float) RAND_MAX - 1) * LIGHT_RADIUS;
                    }

                    vec3 samplePos = light.position + lightRight * offsetX + lightUp * offsetY;

                    if (!checkShadow(hitPos, samplePos, kdtree)) {
                        Light lightSample = light;
                        lightSample.position = samplePos;
                        lightSamples.push_back(lightSample);
                    }
                    // Check for light through portals
                    for (Portal &portal : app.portals) {
                        vec3 transformedLightPos;
                        if (!checkShadowThroughPortal(hitPos, samplePos, portal, kdtree, transformedLightPos)) {
                            Light lightSample = light;
                            lightSample.position = transformedLightPos;
                            lightSamples.push_back(lightSample);
                        }
                    }
                }
            }
            for (const Light &lightSample : lightSamples) {
                vec3 lightDir = normalize(lightSample.position - hitPos);
                vec3 diffuse = material->dif * texColor * std::max(0.f, dot(hitNorm, lightDir)) * light.intensity;
                vec3 H = normalize((lightDir - ray.d) / 2.f);
                vec3 specular = material->spec * std::pow(std::max(0.f, dot(H, hitNorm)), material->shine) * light.intensity * 255.f;
                directLight += diffuse + specular;
            }
            color += directLight / (float) (NUM_SHADOW_SAMPLES_X * NUM_SHADOW_SAMPLES_Y);


        }

        if (bounceDepth < NUM_BOUNCES) {
            vec3 indirectLight(0);
            for (int i = 0; i < NUM_BOUNCE_RAYS / pow(2, bounceDepth); i++) {
                vec3 dir = randomDirInSphere(hitNorm);
                Ray bounceRay(hitPos, dir);
                indirectLight += traceColor(bounceRay, kdtree, bounceDepth + 1);
            }
            color += indirectLight * texColor / 255.f / (float) NUM_BOUNCE_RAYS;
        }

        return color;
    }
    else if (dynamic_cast<Portal *>(hit.tri->obj)) {
        Portal *portal = static_cast<Portal *>(hit.tri->obj);
        if (!portal->open || !portal->linkedPortal->open) {
            if (portal->hasOutline) {
                return portal->outline->color * 255.f;
            }
            else {
                return vec3(0);
            }
        }

        MatrixStack camTransform;
        camTransform.translate(portal->linkedPortal->position);
        camTransform.rotate(M_PI, portal->linkedPortal->getUp());
        camTransform.rotate(portal->linkedPortal->orientation);
        camTransform.rotate(inverse(portal->orientation));
        camTransform.translate(-portal->position);

        vec3 newEye = vec3(camTransform.topMatrix() * vec4(ray.o, 1));
        vec3 newOrig = vec3(camTransform.topMatrix() * vec4(hitPos, 1));
        vec3 newDir = normalize(newOrig - newEye);
        Ray portalRay(newOrig, newDir);
        return traceColor(portalRay, kdtree, bounceDepth);
    }
    else if (dynamic_cast<PortalOutline *>(hit.tri->obj)) {
        return static_cast<PortalOutline *>(hit.tri->obj)->color * 255.f;
    }
    else {
        return vec3(255);
    }
}

void renderRT(int width, int height, const std::string &filename) {
    unsigned char *pixels = new unsigned char[width * height * 3];
    float fov = 45;
    float invHeight = 1.0f / height;
    float invWidth = 1.0f / width;
    float aspect = width * invHeight;
    float angle = tan(M_PI * 0.5 * fov / 180);
    mat4 view = mat4_cast(quatLookAt(app.player.camera.lookAtPoint - app.player.camera.eye, app.player.camera.upVec));

    KdTreeAccel kdtree(app.gameObjects, 80, 1, 0.5, 1, -1);

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspect;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            vec3 dir = normalize(vec3(view * vec4(xx, yy, -1, 0)));
            vec3 orig = app.player.camera.eye;
            Ray ray(orig, dir);
            vec3 pixel = traceColor(ray, kdtree);
            for (int i = 0; i < 3; i++) {
                pixels[(y*width+x)*3+i] = (unsigned char) (std::max(0, std::min(255, (int) round(pixel[i]))));
            }
        }
    }
    stbi_write_png(filename.c_str(), width, height, 3, pixels, width * 3);
    delete[] pixels;
}
