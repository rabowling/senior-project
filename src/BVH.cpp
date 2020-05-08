#include "BVH.h"

#include <glm/glm.hpp>
#include <queue>
#include "GameObject.h"
#include <iostream>

std::atomic<uint32_t> numPrimaryRays(0);
std::atomic<uint32_t> numRayTriangleTests(0);
std::atomic<uint32_t> numRayTriangleIntersections(0);
std::atomic<uint32_t> numRayBBoxTests(0);
std::atomic<uint32_t> numRayBoundingVolumeTests(0);

bool AccelerationStructure::intersect(const glm::vec3& orig, const glm::vec3& dir, RayHit& hit) const
{
    hit.d = kInfinity;
    hit.obj = nullptr;
    const GameObject* intersectedGameObject = nullptr;
    for (const auto& mesh: meshes) {
        RayHit tmpHit;
        tmpHit.obj = mesh;
        if (mesh->intersect(orig, dir, tmpHit.u, tmpHit.v, tmpHit.d, tmpHit.faceIndex) && tmpHit < hit) {
            hit = tmpHit;
        }
    }

    return hit.obj != nullptr;
}

bool BBox::intersect(const glm::vec3& orig, const glm::vec3& invDir, const glm::bvec3& sign, float& tHit) const
{
    numRayBBoxTests++;
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin  = (bounds[sign[0]    ].x - orig.x) * invDir.x;
    tmax  = (bounds[1 - sign[0]].x - orig.x) * invDir.x;
    tymin = (bounds[sign[1]    ].y - orig.y) * invDir.y;
    tymax = (bounds[1 - sign[1]].y - orig.y) * invDir.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[sign[2]    ].z - orig.z) * invDir.z;
    tzmax = (bounds[1 - sign[2]].z - orig.z) * invDir.z;

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    tHit = tmin;

    return true;
}

const glm::vec3 BVH::planeSetNormals[BVH::kNumPlaneSetNormals] = {
    glm::vec3(1, 0, 0),
    glm::vec3(0, 1, 0),
    glm::vec3(0, 0, 1),
    glm::vec3( sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
    glm::vec3( sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f)
};

BVH::BVH(std::list<GameObject *>& m) : AccelerationStructure(m)
{
    Extents sceneExtents; // that's the extent of the entire scene which we need to compute for the octree
    extentsList.reserve(meshes.size());
    uint32_t i = 0;
    for (auto mesh = meshes.begin(); mesh != meshes.end(); mesh++, i++) {
        for (uint8_t j = 0; j < kNumPlaneSetNormals; ++j) {
            for (auto v = (*mesh)->posBufCache.begin(); v < (*mesh)->posBufCache.end(); v += 3) {
                glm::vec3 vtx(v[0], v[1], v[2]);
                float d = dot(planeSetNormals[j], vtx);
                // set dNEar and dFar
                if (d < extentsList[i].d[j][0]) extentsList[i].d[j][0] = d;
                if (d > extentsList[i].d[j][1]) extentsList[i].d[j][1] = d;
            }
        }
        sceneExtents.extendBy(extentsList[i]); // expand the scene extent of this object's extent
        extentsList[i].mesh = *mesh; // the extent itself needs to keep a pointer to the object its holds
    }

    // Now that we have the extent of the scene we can start building our octree
    // Using C++ make_unique function here but you don't need to, just to learn something...
    octree = new Octree(sceneExtents);

    for (uint32_t i = 0; i < meshes.size(); ++i) {
        octree->insert(&extentsList[i]);
    }

    // Build from bottom up
    octree->build();
}

bool BVH::Extents::intersect(
    const float* precomputedNumerator,
    const float* precomputedDenominator,
    float& tNear,   // tn and tf in this method need to be contained
    float& tFar,    // within the range [tNear:tFar]
    uint8_t& planeIndex) const
{
    numRayBoundingVolumeTests++;
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        float tNearExtents = (d[i][0] - precomputedNumerator[i]) / precomputedDenominator[i];
        float tFarExtents = (d[i][1] - precomputedNumerator[i]) / precomputedDenominator[i];
        if (precomputedDenominator[i] < 0) std::swap(tNearExtents, tFarExtents);
        if (tNearExtents > tNear) tNear = tNearExtents, planeIndex = i;
        if (tFarExtents < tFar) tFar = tFarExtents;
        if (tNear > tFar) return false;
    }

    return true;
}

bool BVH::intersect(const glm::vec3& orig, const glm::vec3& dir, RayHit& hit) const
{
    hit.d = kInfinity;
    hit.obj = nullptr;
    float tHit = kInfinity;
    const GameObject* intersectedGameObject = nullptr;
    float precomputedNumerator[BVH::kNumPlaneSetNormals];
    float precomputedDenominator[BVH::kNumPlaneSetNormals];
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        precomputedNumerator[i] = dot(planeSetNormals[i], orig);
        precomputedDenominator[i] = dot(planeSetNormals[i], dir);
    }

    uint8_t planeIndex;
    float tNear = 0, tFar = kInfinity; // tNear, tFar for the intersected extents
    if (!octree->root->nodeExtents.intersect(precomputedNumerator, precomputedDenominator, tNear, tFar, planeIndex) || tFar < 0)
        return false;
    tHit = tFar;
    std::priority_queue<BVH::Octree::QueueElement> queue;
    queue.push(BVH::Octree::QueueElement(octree->root, 0));
    while (!queue.empty() && queue.top().t < tHit) {
        const Octree::OctreeNode *node = queue.top().node;
        queue.pop();
        if (node->isLeaf) {
            for (const auto& e: node->nodeExtentsList) {
                float t = kInfinity;
                RayHit tmpHit;
                tmpHit.obj = const_cast<GameObject *>(e->mesh);
                numPrimaryRays++;
                if (e->mesh->intersect(orig, dir, tmpHit.u, tmpHit.v, tmpHit.d, tmpHit.faceIndex) && tmpHit < hit) {
                    tHit = t;
                    hit = tmpHit;
                }
            }
        }
        else {
            for (uint8_t i = 0; i < 8; ++i) {
                if (node->child[i] != nullptr) {
                    float tNearChild = 0, tFarChild = tFar;
                    if (node->child[i]->nodeExtents.intersect(precomputedNumerator, precomputedDenominator, tNearChild, tFarChild, planeIndex)) {
                        float t = (tNearChild < 0 && tFarChild >= 0) ? tFarChild : tNearChild;
                        queue.push(BVH::Octree::QueueElement(node->child[i], t));
                    }
                }
            }
        }
    }
    return hit.obj != nullptr;
}

bool BVH::checkBlocked(const glm::vec3& orig, const glm::vec3& dir, float d) const {
    float tHit = d;
    const GameObject* intersectedGameObject = nullptr;
    float precomputedNumerator[BVH::kNumPlaneSetNormals];
    float precomputedDenominator[BVH::kNumPlaneSetNormals];
    for (uint8_t i = 0; i < kNumPlaneSetNormals; ++i) {
        precomputedNumerator[i] = dot(planeSetNormals[i], orig);
        precomputedDenominator[i] = dot(planeSetNormals[i], dir);
    }

    uint8_t planeIndex;
    float tNear = 0, tFar = kInfinity; // tNear, tFar for the intersected extents
    if (!octree->root->nodeExtents.intersect(precomputedNumerator, precomputedDenominator, tNear, tFar, planeIndex) || tFar < 0)
        return false;
    tHit = tFar;
    std::priority_queue<BVH::Octree::QueueElement> queue;
    queue.push(BVH::Octree::QueueElement(octree->root, 0));
    while (!queue.empty() && queue.top().t < tHit) {
        const Octree::OctreeNode *node = queue.top().node;
        queue.pop();
        if (node->isLeaf) {
            for (const auto& e: node->nodeExtentsList) {
                float t = kInfinity;
                RayHit tmpHit;
                if (e->mesh->intersect(orig, dir, tmpHit.u, tmpHit.v, tmpHit.d, tmpHit.faceIndex) && tmpHit.d < d) {
                    return true;
                }
            }
        }
        else {
            for (uint8_t i = 0; i < 8; ++i) {
                if (node->child[i] != nullptr) {
                    float tNearChild = 0, tFarChild = tFar;
                    if (node->child[i]->nodeExtents.intersect(precomputedNumerator, precomputedDenominator, tNearChild, tFarChild, planeIndex)) {
                        float t = (tNearChild < 0 && tFarChild >= 0) ? tFarChild : tNearChild;
                        queue.push(BVH::Octree::QueueElement(node->child[i], t));
                    }
                }
            }
        }
    }
    return false;
}
