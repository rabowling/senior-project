#pragma once

#include <memory>
#include <list>
#include <glm/glm.hpp>
#include "GameObject.h"

// https://blog.frogslayer.com/kd-trees-for-faster-ray-tracing-with-triangles/


struct RayHit {
    float d;
    float u, v;
    unsigned int faceIndex;
    GameObject *obj;
    bool operator<(const RayHit &rhs);
    bool operator>(const RayHit &rhs) { return !operator<(rhs); }
};

struct BBox {
    BBox();
    glm::vec3 bbMin, bbMax;
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, float d = 0);
};

struct Triangle {
    GameObject *obj;
    glm::vec3 verts[3];
    int faceIndex;
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit) const;
};

class KDNode {
public:
    static std::unique_ptr<KDNode> build(const std::list<GameObject *> &gameObjects);
    static std::unique_ptr<KDNode> build(const std::vector<Triangle> &tris, int depth);
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit);
    bool checkBlocked(const glm::vec3 &orig, const glm::vec3 &dir, float d);

private:
    std::unique_ptr<KDNode> left = nullptr;
    std::unique_ptr<KDNode> right = nullptr;
    BBox bbox;
    std::vector<Triangle> tris;
};