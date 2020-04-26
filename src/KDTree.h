#pragma once

#include <memory>
#include <list>
#include <glm/glm.hpp>
#include "GameObject.h"

// https://blog.frogslayer.com/kd-trees-for-faster-ray-tracing-with-triangles/

#define COST_TRAVERSE 1.0
#define COST_INTERSECT 1.5

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
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &tmin, float &tmax, float d = 0);

    float dx() const {
        return bbMax.x - bbMin.x;
    };
    float dy() const {
        return bbMax.y - bbMin.y;
    };
    float dz() const {
        return bbMax.z - bbMin.z;
    };
    float d(int axis) const {
        return bbMax[axis] - bbMin[axis];
    }
    float isPlanar(){
        return dx() <= 0.01 || dy() <= 0.01 || dz() <= 0.01;
    }
};

struct Triangle {
    GameObject *obj;
    glm::vec3 verts[3];
    int faceIndex;
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit) const;
    BBox getBounds() const;
};

struct SplitPlane {
    SplitPlane(const int axis, const float pos): axis(axis), pos(pos){};
    SplitPlane(){};
    int axis; // 0=x, 1=y, 2=z
    float pos;
    bool operator==(const SplitPlane& sp) {
        return(axis == sp.axis && pos == sp.pos);
    }
};

// https://github.com/arvearve/Raytracer/blob/master/BasicRayTracer/kdTree.cpp
class KDNode {
public:
    static std::unique_ptr<KDNode> build(const std::list<GameObject *> &gameObjects);
    bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit);
    bool checkBlocked(const glm::vec3 &orig, const glm::vec3 &dir, float d);

private:
    enum PlaneSide { LEFT=-1, RIGHT=1, UNKNOWN=0 };
    static bool isDone(int N, float minCv);
    static void splitBox(const BBox &V, const SplitPlane &p, BBox &VL, BBox &VR);
    static void SAH(const SplitPlane &p, const BBox &V, int NL, int NR, int NP, float &CP, PlaneSide& pside);
    static void findPlane(const std::vector<Triangle> &T, const BBox &V, int depth, SplitPlane &pEst, float &cEst, PlaneSide &psideEst);
    static void sortTriangles(const std::vector<Triangle> &T, const SplitPlane &p, const PlaneSide &pside, std::vector<Triangle> &TL, std::vector<Triangle> &TR);
    static std::unique_ptr<KDNode> recBuild(const std::vector<Triangle> &tris, const BBox &V, int depth, const SplitPlane &prevPlane);
    bool recIntersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit, float tmin, float tmax);

    std::unique_ptr<KDNode> left = nullptr;
    std::unique_ptr<KDNode> right = nullptr;
    bool leaf = false;
    SplitPlane plane;
    BBox bbox;
    std::vector<Triangle> tris;
};