#pragma once

#include <memory>
#include <list>
#include <glm/glm.hpp>
#include "GameObject.h"

// http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Kd-Tree_Accelerator.html

struct Ray {
    Ray(glm::vec3 o, glm::vec3 d);
    Ray(glm::vec3 o, glm::vec3 d, float tMax);
    glm::vec3 o, d;
    float tMax;
};

struct Bounds3f {
public:
    void extend(const Bounds3f &other);
    float SurfaceArea() const;
    int MaximumExtent() const;
    bool IntersectP(const Ray &ray, float *hitt0, float *hitt1) const;
    glm::vec3 pMin, pMax;
};

struct SurfaceInteraction {
    float d;
    float u, v;
    unsigned int faceIndex;
    GameObject *obj;
    bool operator<(const SurfaceInteraction &rhs);
    bool operator>(const SurfaceInteraction &rhs) { return !operator<(rhs); }
};

class Primitive {
public:
    GameObject *obj;
    glm::vec3 verts[3];
    int faceIndex;
    bool Intersect(const Ray &r, SurfaceInteraction &si) const;
    bool IntersectP(const Ray &r) const;
    Bounds3f WorldBound() const;
};

struct KdAccelNode;
struct BoundEdge;
class KdTreeAccel {
  public:
    // KdTreeAccel Public Methods
    KdTreeAccel(const std::list<GameObject *> &gameObjects,
                int isectCost = 80, int traversalCost = 1,
                float emptyBonus = 0.5, int maxPrims = 1, int maxDepth = -1);
    Bounds3f WorldBound() const { return bounds; }
    ~KdTreeAccel();
    bool Intersect(const Ray &ray, SurfaceInteraction &isect) const;
    bool IntersectP(const Ray &ray) const;

  private:
    // KdTreeAccel Private Methods
    void buildTree(int nodeNum, const Bounds3f &bounds,
                   const std::vector<Bounds3f> &primBounds, int *primNums,
                   int nprims, int depth,
                   const std::unique_ptr<BoundEdge[]> edges[3], int *prims0,
                   int *prims1, int badRefines = 0);

    // KdTreeAccel Private Data
    const int isectCost, traversalCost, maxPrims;
    const float emptyBonus;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<int> primitiveIndices;
    KdAccelNode *nodes;
    int nAllocedNodes, nextFreeNode;
    Bounds3f bounds;
};

struct KdToDo {
    const KdAccelNode *node;
    float tMin, tMax;
};
