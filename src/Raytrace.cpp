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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace glm;
using namespace std;

glm::vec3 traceColor(const Ray &ray, const KdTreeAccel &kdtree) {
    SurfaceInteraction hit;
    if (!kdtree.Intersect(ray, hit)) {
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

    vec3 hitPos = hit.u * vert[1] + hit.v * vert[2] + (1 - hit.u - hit.v) * vert[0];

    Material *material = hit.obj->getMaterial();
    if (material) {
        Texture *texture = material->getTexture();
        vec2 uv = hit.u * vt[1] + hit.v * vt[2] + (1 - hit.u - hit.v) * vt[0];

        // scale UV for Wall objects
        if (dynamic_cast<Wall *>(hit.obj)) {
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
        vec3 normal = normalize(cross(vert[1] - vert[0], vert[2] - vert[0]));
        for (const Light &light : app.lights) {
            // Blinn-Phong shading
            vec3 ambient = material->amb * texColor * light.intensity;
            color += ambient;

            // Shadow rays
            SurfaceInteraction shadowRayHit;
            Ray shadowRay(hitPos, normalize(light.position - hitPos));
            if (!kdtree.Intersect(shadowRay, shadowRayHit)
                    || shadowRayHit.d > distance(light.position, hitPos)) {
                vec3 lightDir = normalize(light.position - hitPos);
                vec3 diffuse = material->dif * texColor * std::max(0.f, dot(normal, lightDir)) * light.intensity;
                vec3 H = normalize((lightDir - ray.d) / 2.f);
                vec3 specular = material->spec * std::pow(std::max(0.f, dot(H, normal)), material->shine) * light.intensity * 255.f;
                color += diffuse + specular;
            }

            // Check for light through portals
            for (Portal &portal : app.portals) {
                if (!(portal.facing(hitPos) && portal.linkedPortal->facing(light.position))) {
                    continue;
                }

                MatrixStack camTransform2;
                camTransform2.translate(portal.position);
                camTransform2.rotate(M_PI, portal.getUp());
                camTransform2.rotate(portal.orientation);
                camTransform2.rotate(inverse(portal.linkedPortal->orientation));
                camTransform2.translate(-portal.linkedPortal->position);

                vec3 newLightPos = vec3(camTransform2.topMatrix() * vec4(light.position, 1));
                Ray portalRay(hitPos, normalize(newLightPos - hitPos));
                if (kdtree.Intersect(portalRay, shadowRayHit)
                        && shadowRayHit.obj == &portal) {
                    vec3 vert2[3];
                    Shape *model2 = shadowRayHit.obj->getModel();
                    for (int vNum = 0; vNum < 3; vNum++) {
                        unsigned int vIdx = model2->eleBuf[shadowRayHit.faceIndex*3+vNum];
                        for (int i = 0; i < 3; i++) {
                            vert2[vNum][i] = shadowRayHit.obj->posBufCache[vIdx*3+i];
                        }
                    }

                    MatrixStack camTransform;
                    camTransform.translate(portal.linkedPortal->position);
                    camTransform.rotate(M_PI, portal.linkedPortal->getUp());
                    camTransform.rotate(portal.linkedPortal->orientation);
                    camTransform.rotate(inverse(portal.orientation));
                    camTransform.translate(-portal.position);

                    vec3 shadowHitPos = shadowRayHit.u * vert2[1] + shadowRayHit.v * vert2[2] + (1 - shadowRayHit.u - shadowRayHit.v) * vert2[0];
                    vec3 shadowEye = vec3(camTransform.topMatrix() * vec4(hitPos, 1));
                    vec3 shadowOrig = vec3(camTransform.topMatrix() * vec4(shadowHitPos, 1));
                    vec3 shadowDir = normalize(shadowOrig - shadowEye);
                    float d = distance(light.position, shadowOrig);
                    Ray portalShadowRay(shadowOrig, shadowDir, d);
                    if (!kdtree.IntersectP(portalShadowRay)) {
                        vec3 lightDir = normalize(newLightPos - hitPos);
                        vec3 diffuse = material->dif * texColor * std::max(0.f, dot(normal, lightDir)) * light.intensity;
                        vec3 H = normalize((lightDir - ray.d) / 2.f);
                        vec3 specular = material->spec * std::pow(std::max(0.f, dot(H, normal)), material->shine) * light.intensity * 255.f;
                        color = color + diffuse + specular;
                    }
                }
            }

        }

        return color;
    }
    else if (dynamic_cast<Portal *>(hit.obj)) {
        Portal *portal = static_cast<Portal *>(hit.obj);

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
        return traceColor(portalRay, kdtree);
    }
    else if (dynamic_cast<PortalOutline *>(hit.obj)) {
        return static_cast<PortalOutline *>(hit.obj)->color * 255.f;
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

    KdTreeAccel kdtree(app.gameObjects);
    int count = 0;

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (++count % 1000 == 0) {
                cout << count << endl;
            }
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
