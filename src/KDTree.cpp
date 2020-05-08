#include "KDTree.h"
#include "Portal.h"
#include "PortalOutline.h"
#include <glm/glm.hpp>
#include <algorithm>
#include "Box.h"

using namespace glm;
using namespace std;

#define EPSILON 0.00001

bool SurfaceInteraction::operator<(const SurfaceInteraction &rhs) {
    if (abs(d - rhs.d) < 0.0001) {
        if (dynamic_cast<Portal *>(tri->obj)) {
            return true;
        }
        else if (dynamic_cast<Portal *>(rhs.tri->obj)) {
            return false;
        }
        else if (dynamic_cast<PortalOutline *>(tri->obj)) {
            return true;
        }
        else if (dynamic_cast<PortalOutline *>(rhs.tri->obj)) {
            return false;
        }
    }
    return d < rhs.d;
}

Ray::Ray(glm::vec3 o, glm::vec3 d) : o(o), d(d), tMax(INFINITY) {

}

Ray::Ray(glm::vec3 o, glm::vec3 d, float tMax) : o(o), d(d), tMax(tMax) {

}

bool Bounds3f::IntersectP(const Ray &ray, float *hitt0, float *hitt1) const {
    float t0 = 0, t1 = ray.tMax;
    for (int i = 0; i < 3; ++i) {
        float invRayDir = 1 / ray.d[i];
        float tNear = (pMin[i] - ray.o[i]) * invRayDir;
        float tFar  = (pMax[i] - ray.o[i]) * invRayDir;
        if (tNear > tFar) std::swap(tNear, tFar);
        tFar *= 1 + 2 * tgamma(3);

        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar  < t1 ? tFar  : t1;
        if (t0 > t1) return false;

    }
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;
}

void Bounds3f::extend(const Bounds3f &other) {
    pMin = glm::min(pMin, other.pMin);
    pMax = glm::max(pMax, other.pMax);
}

int Bounds3f::MaximumExtent() const {
    glm::vec3 d = pMax - pMin;
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}

float Bounds3f::SurfaceArea() const {
    vec3 d = pMax - pMin;
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

Bounds3f Primitive::WorldBound() const {
    Bounds3f b;
    b.pMin = glm::min(verts[0], glm::min(verts[1], verts[2]));
    b.pMax = glm::max(verts[0], glm::max(verts[1], verts[2]));
    return b;
}


// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool Primitive::Intersect(const Ray &r, SurfaceInteraction &si) const {
    vec3 edge1 = verts[1] - verts[0];
    vec3 edge2 = verts[2] - verts[0];
    vec3 pvec = cross(r.d, edge2);
    float det = dot(edge1, pvec);

    if (det < EPSILON) {
        return false;
    }

    vec3 tvec = r.o - verts[0];
    si.u = dot(tvec, pvec);
    if (si.u < 0 || si.u > det) {
        return false;
    }

    vec3 qvec = cross(tvec, edge1);
    si.v = dot(r.d, qvec);
    if (si.v < 0 || si.u + si.v > det) {
        return false;
    }

    si.d = dot(edge2, qvec);
    float inv_det = 1 / det;
    si.d *= inv_det;
    si.u *= inv_det;
    si.v *= inv_det;

    return si.d > -EPSILON;
}

bool Primitive::IntersectP(const Ray &r) const {
    SurfaceInteraction si;
    return Intersect(r, si) && si.d <= r.tMax;
}

struct KdAccelNode {
    // KdAccelNode Methods
    void InitLeaf(int *primNums, int np, std::vector<int> *primitiveIndices);
    void InitInterior(int axis, int ac, float s) {
        split = s;
        flags = axis;
        aboveChild |= (ac << 2);
    }
    float SplitPos() const { return split; }
    int nPrimitives() const { return nPrims >> 2; }
    int SplitAxis() const { return flags & 3; }
    bool IsLeaf() const { return (flags & 3) == 3; }
    int AboveChild() const { return aboveChild >> 2; }
    union {
        float split;                 // Interior
        int onePrimitive;            // Leaf
        int primitiveIndicesOffset;  // Leaf
    };

  private:
    union {
        int flags;       // Both
        int nPrims;      // Leaf
        int aboveChild;  // Interior
    };
};

enum class EdgeType { Start, End };
struct BoundEdge {
    // BoundEdge Public Methods
    BoundEdge() {}
    BoundEdge(float t, int primNum, bool starting) : t(t), primNum(primNum) {
        type = starting ? EdgeType::Start : EdgeType::End;
    }
    float t;
    int primNum;
    EdgeType type;
};

void gameObjectToPrimitives(GameObject *obj, const mat4 &transform, std::vector<std::shared_ptr<Primitive>> &tris) {
    Shape *model = obj->getModel();
    if (obj->posBufCache.size() != model->posBuf.size()) {
        obj->posBufCache.resize(model->posBuf.size());
    }

    // loop over each face
    for (int fIdx = 0; fIdx < model->eleBuf.size() / 3; fIdx++) {
        shared_ptr<Primitive> tri = make_shared<Primitive>();
        tri->faceIndex = fIdx;
        tri->obj = obj;

        for (int vNum = 0; vNum < 3; vNum++) {
            // get transformed vertex coordinates
            unsigned int vIdx = model->eleBuf[fIdx*3+vNum];
            for (int i = 0; i < 3; i++) {
                tri->verts[vNum][i] = model->posBuf[vIdx*3+i];
            }
            tri->verts[vNum] = vec3(transform * vec4(tri->verts[vNum], 1));

            // cache transformed vertex coordinates
            for (int i = 0; i < 3; i++) {
                obj->posBufCache[vIdx*3+i] = tri->verts[vNum][i];
            }
        }
        tris.push_back(tri);
    }
}

// KdTreeAccel Method Definitions
std::vector<std::shared_ptr<Primitive>> gameObjectsToPrimitives(const std::list<GameObject *> &gameObjects) {
    std::vector<std::shared_ptr<Primitive>> tris;

    for (GameObject *obj : gameObjects) {
        mat4 transform = obj->getTransform();
        gameObjectToPrimitives(obj, transform, tris);
        if (dynamic_cast<Box *>(obj)) {
            Box *box = static_cast<Box *>(obj);
            for (Portal *portal : box->touchingPortals) {
                mat4 boxTransform = portal->getTransformToLinkedPortal() * transform;
                gameObjectToPrimitives(box, boxTransform, tris);
            }
        }
    }

    return tris;
}

KdTreeAccel::KdTreeAccel(const std::list<GameObject *> &gameObjects,
                         int isectCost, int traversalCost, float emptyBonus,
                         int maxPrims, int maxDepth)
    : isectCost(isectCost),
      traversalCost(traversalCost),
      maxPrims(maxPrims),
      emptyBonus(emptyBonus),
      primitives(gameObjectsToPrimitives(gameObjects)) {
    // Build kd-tree for accelerator
    nextFreeNode = nAllocedNodes = 0;
    if (maxDepth <= 0)
        maxDepth = std::round(8 + 1.3f * ceil(log(int64_t(primitives.size()))));

    // Compute bounds for kd-tree construction
    std::vector<Bounds3f> primBounds;
    primBounds.reserve(primitives.size());
    for (const std::shared_ptr<Primitive> &prim : primitives) {
        Bounds3f b = prim->WorldBound();
        bounds.extend(b);
        primBounds.push_back(b);
    }

    // Allocate working memory for kd-tree construction
    std::unique_ptr<BoundEdge[]> edges[3];
    for (int i = 0; i < 3; ++i)
        edges[i].reset(new BoundEdge[2 * primitives.size()]);
    std::unique_ptr<int[]> prims0(new int[primitives.size()]);
    std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * primitives.size()]);

    // Initialize _primNums_ for kd-tree construction
    std::unique_ptr<int[]> primNums(new int[primitives.size()]);
    for (size_t i = 0; i < primitives.size(); ++i) primNums[i] = i;

    // Start recursive construction of kd-tree
    buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
              maxDepth, edges, prims0.get(), prims1.get());
}

void KdAccelNode::InitLeaf(int *primNums, int np,
                           std::vector<int> *primitiveIndices) {
    flags = 3;
    nPrims |= (np << 2);
    // Store primitive ids for leaf node
    if (np == 0)
        onePrimitive = 0;
    else if (np == 1)
        onePrimitive = primNums[0];
    else {
        primitiveIndicesOffset = primitiveIndices->size();
        for (int i = 0; i < np; ++i) primitiveIndices->push_back(primNums[i]);
    }
}

KdTreeAccel::~KdTreeAccel() { delete[] nodes; }

void KdTreeAccel::buildTree(int nodeNum, const Bounds3f &nodeBounds,
                            const std::vector<Bounds3f> &allPrimBounds,
                            int *primNums, int nPrimitives, int depth,
                            const std::unique_ptr<BoundEdge[]> edges[3],
                            int *prims0, int *prims1, int badRefines) {
    // Get next free node from _nodes_ array
    if (nextFreeNode == nAllocedNodes) {
        int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
        KdAccelNode *n = new KdAccelNode[nNewAllocNodes];
        if (nAllocedNodes > 0) {
            memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
            delete[] nodes;
        }
        nodes = n;
        nAllocedNodes = nNewAllocNodes;
    }
    ++nextFreeNode;

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    float bestCost = INFINITY;
    float oldCost = isectCost * float(nPrimitives);
    float totalSA = nodeBounds.SurfaceArea();
    float invTotalSA = 1 / totalSA;
    glm::vec3 d = nodeBounds.pMax - nodeBounds.pMin;

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;
retrySplit:

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const Bounds3f &bounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(bounds.pMin[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(bounds.pMax[axis], pn, false);
    }

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int)e0.type < (int)e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;
    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) --nAbove;
        float edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.pMin[axis] && edgeT < nodeBounds.pMax[axis]) {
            // Compute cost for split at _i_th edge

            // Compute child surface areas for split at _edgeT_
            int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
            float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                 (edgeT - nodeBounds.pMin[axis]) *
                                     (d[otherAxis0] + d[otherAxis1]));
            float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                 (nodeBounds.pMax[axis] - edgeT) *
                                     (d[otherAxis0] + d[otherAxis1]));
            float pBelow = belowSA * invTotalSA;
            float pAbove = aboveSA * invTotalSA;
            float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;
            float cost =
                traversalCost +
                isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

            // Update best split if this is lowest cost so far
            if (cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestOffset = i;
            }
        }
        if (edges[axis][i].type == EdgeType::Start) ++nBelow;
    }

    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2) {
        ++retries;
        axis = (axis + 1) % 3;
        goto retrySplit;
    }
    if (bestCost > oldCost) ++badRefines;
    if ((bestCost > 4 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Classify primitives with respect to split
    int n0 = 0, n1 = 0;
    for (int i = 0; i < bestOffset; ++i)
        if (edges[bestAxis][i].type == EdgeType::Start)
            prims0[n0++] = edges[bestAxis][i].primNum;
    for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
        if (edges[bestAxis][i].type == EdgeType::End)
            prims1[n1++] = edges[bestAxis][i].primNum;

    // Recursively initialize children nodes
    float tSplit = edges[bestAxis][bestOffset].t;
    Bounds3f bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.pMax[bestAxis] = bounds1.pMin[bestAxis] = tSplit;
    buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines);
    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines);
}

bool KdTreeAccel::Intersect(const Ray &ray, SurfaceInteraction &isect) const {
    // Compute initial parametric range of ray inside kd-tree extent
    float tMin, tMax;
    if (!bounds.IntersectP(ray, &tMin, &tMax)) {
        return false;
    }

    // Prepare to traverse kd-tree for ray
    glm::vec3 invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    const int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;

    // Traverse kd-tree nodes in order for ray
    bool hit = false;
    const KdAccelNode *node = &nodes[0];
    while (node != nullptr) {
        // Bail out if we found a hit closer than the current node
        if (hit && isect.d < tMin) break;
        if (!node->IsLeaf()) {
            // Process kd-tree interior node

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            float tPlane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =
                (ray.o[axis] < node->SplitPos()) ||
                (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
            if (belowFirst) {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            } else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in todo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
        } else {
            // Check for intersections inside leaf node
            int nPrimitives = node->nPrimitives();
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p =
                    primitives[node->onePrimitive];
                // Check one primitive inside leaf node
                SurfaceInteraction newIsect;
                newIsect.tri = p.get();
                if (p->Intersect(ray, newIsect)) {
                    if (hit) {
                        if (newIsect < isect) {
                            isect = newIsect;
                        }
                    }
                    else {
                        isect = newIsect;
                        hit = true;
                    }
                }
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int index =
                        primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &p = primitives[index];
                    // Check one primitive inside leaf node
                    SurfaceInteraction newIsect;
                    newIsect.tri = p.get();
                    if (p->Intersect(ray, newIsect)) {
                        if (hit) {
                            if (newIsect < isect) {
                                isect = newIsect;
                            }
                        }
                        else {
                            isect = newIsect;
                            hit = true;
                        }
                    }
                }
            }

            // Grab next node to process from todo list
            if (todoPos > 0) {
                --todoPos;
                node = todo[todoPos].node;
                tMin = todo[todoPos].tMin;
                tMax = todo[todoPos].tMax;
            } else
                break;
        }
    }
    return hit;
}

bool KdTreeAccel::IntersectP(const Ray &ray) const {
    // Compute initial parametric range of ray inside kd-tree extent
    float tMin, tMax;
    if (!bounds.IntersectP(ray, &tMin, &tMax)) {
        return false;
    }

    // Prepare to traverse kd-tree for ray
    glm::vec3 invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    const int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;
    const KdAccelNode *node = &nodes[0];
    while (node != nullptr) {
        if (node->IsLeaf()) {
            // Check for shadow ray intersections inside leaf node
            int nPrimitives = node->nPrimitives();
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p =
                    primitives[node->onePrimitive];
                if (p->IntersectP(ray)) {
                    return true;
                }
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int primitiveIndex =
                        primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &prim =
                        primitives[primitiveIndex];
                    if (prim->IntersectP(ray)) {
                        return true;
                    }
                }
            }

            // Grab next node to process from todo list
            if (todoPos > 0) {
                --todoPos;
                node = todo[todoPos].node;
                tMin = todo[todoPos].tMin;
                tMax = todo[todoPos].tMax;
            } else
                break;
        } else {
            // Process kd-tree interior node

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            float tPlane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =
                (ray.o[axis] < node->SplitPos()) ||
                (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
            if (belowFirst) {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            } else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in todo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
        }
    }
    return false;
}
