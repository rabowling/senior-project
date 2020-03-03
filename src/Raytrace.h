#pragma once

#include "GameObject.h"
#include <string>
#include <glm/glm.hpp>

struct RayHit {
    float d;
    float u, v;
    unsigned int faceIndex;
    GameObject *obj;
};

// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool intersectRayTriangle(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 &v0, const glm::vec3 &v1,
    const glm::vec3 &v2, float &d, float &u, float &v);

bool intersectRayShape(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<float> &posBuf, const std::vector<unsigned int> eleBuf,
    RayHit &closestHit);
void renderRT(int width, int height, const std::string &filename);