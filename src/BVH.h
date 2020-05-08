#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <limits>
#include "GameObject.h"
#include <list>
#include "Ray.h"

// https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/bounding-volume-hierarchy-BVH-part2

const float kEpsilon = 1e-8;
const float kInfinity = std::numeric_limits<float>::max();

class BBox
{
public:
    BBox() {}
    BBox(glm::vec3 min_, glm::vec3 max_)
    {
        bounds[0] = min_;
        bounds[1] = max_;
    }
    BBox& extendBy(const glm::vec3& p)
    {
        if (p.x < bounds[0].x) bounds[0].x = p.x;
        if (p.y < bounds[0].y) bounds[0].y = p.y;
        if (p.z < bounds[0].z) bounds[0].z = p.z;
        if (p.x > bounds[1].x) bounds[1].x = p.x;
        if (p.y > bounds[1].y) bounds[1].y = p.y;
        if (p.z > bounds[1].z) bounds[1].z = p.z;

        return *this;
    }
    /*inline */ glm::vec3 centroid() const { return (bounds[0] + bounds[1]) * 0.5f; }
    glm::vec3& operator [] (bool i) { return bounds[i]; }
    const glm::vec3 operator [] (bool i) const { return bounds[i]; }
    bool intersect(const glm::vec3&, const glm::vec3&, const glm::bvec3&, float&) const;
    glm::vec3 bounds[2] = { glm::vec3(kInfinity), glm::vec3(-kInfinity) };
};

class AccelerationStructure
{
public:
    AccelerationStructure(std::list<GameObject *>& m) : meshes(m) {}
    virtual ~AccelerationStructure() {}
    virtual bool intersect(const glm::vec3& orig, const glm::vec3& dir, RayHit& hit) const;
protected:
    const std::list<GameObject *> meshes;
};

class BVH : public AccelerationStructure
{
    static const uint8_t kNumPlaneSetNormals = 7;
    static const glm::vec3 planeSetNormals[kNumPlaneSetNormals];
    struct Extents
    {
        Extents()
        {
            for (uint8_t i = 0;  i < kNumPlaneSetNormals; ++i)
                d[i][0] = kInfinity, d[i][1] = -kInfinity;
        }
        void extendBy(const Extents& e)
        {

            for (uint8_t i = 0;  i < kNumPlaneSetNormals; ++i) {
                if (e.d[i][0] < d[i][0]) d[i][0] = e.d[i][0];
                if (e.d[i][1] > d[i][1]) d[i][1] = e.d[i][1];
            }
        }
        /* inline */
        glm::vec3 centroid() const
        {
            return glm::vec3(
                d[0][0] + d[0][1] * 0.5,
                d[1][0] + d[1][1] * 0.5,
                d[2][0] + d[2][1] * 0.5);
        }
        bool intersect(const float*, const float*, float&, float&, uint8_t&) const;
        float d[kNumPlaneSetNormals][2];
        const GameObject* mesh;
    };

    struct Octree
    {
        Octree(const Extents& sceneExtents)
        {
            float xDiff = sceneExtents.d[0][1] - sceneExtents.d[0][0];
            float yDiff = sceneExtents.d[1][1] - sceneExtents.d[1][0];
            float zDiff = sceneExtents.d[2][1] - sceneExtents.d[2][0];
            float maxDiff = std::max(xDiff, std::max(yDiff, zDiff));
            glm::vec3 minPlusMax(
                sceneExtents.d[0][0] + sceneExtents.d[0][1],
                sceneExtents.d[1][0] + sceneExtents.d[1][1],
                sceneExtents.d[2][0] + sceneExtents.d[2][1]);
            bbox[0] = (minPlusMax - maxDiff) * 0.5f;
            bbox[1] = (minPlusMax + maxDiff) * 0.5f;
            root = new OctreeNode;
        }

        ~Octree() { deleteOctreeNode(root); }

        void insert(const Extents* extents) { insert(root, extents, bbox, 0); }
        void build() { build(root, bbox); };

        struct OctreeNode
        {
            OctreeNode* child[8] = { nullptr };
            std::vector<const Extents *> nodeExtentsList; // pointer to the objects extents
            Extents nodeExtents; // extents of the octree node itself
            bool isLeaf = true;
        };

        struct QueueElement
        {
            const OctreeNode *node; // octree node held by this element in the queue
            float t; // distance from the ray origin to the extents of the node
            QueueElement(const OctreeNode *n, float tn) : node(n), t(tn) {}
            // priority_queue behaves like a min-heap
            friend bool operator < (const QueueElement &a, const QueueElement &b) { return a.t > b.t; }
        };

        OctreeNode* root = nullptr; // make unique son don't have to manage deallocation
        BBox bbox;

    private:

        void deleteOctreeNode(OctreeNode*& node)
        {
            for (uint8_t i = 0; i < 8; i++) {
                if (node->child[i] != nullptr) {
                    deleteOctreeNode(node->child[i]);
                }
            }
            delete node;
        }

        void insert(OctreeNode*& node, const Extents* extents, const BBox& bbox, uint32_t depth)
        {
            if (node->isLeaf) {
                if (node->nodeExtentsList.size() == 0 || depth == 16) {
                    node->nodeExtentsList.push_back(extents);
                }
                else {
                    node->isLeaf = false;
                    // Re-insert extents held by this node
                    while (node->nodeExtentsList.size()) {
                        insert(node, node->nodeExtentsList.back(), bbox, depth);
                        node->nodeExtentsList.pop_back();
                    }
                    // Insert new extent
                    insert(node, extents, bbox, depth);
                }
            }
            else {
                // Need to compute in which child of the current node this extents should
                // be inserted into
                glm::vec3 extentsCentroid = extents->centroid();
                glm::vec3 nodeCentroid = (bbox[0] + bbox[1]) * 0.5f;
                BBox childBBox;
                uint8_t childIndex = 0;
                // x-axis
                if (extentsCentroid.x > nodeCentroid.x) {
                    childIndex = 4;
                    childBBox[0].x = nodeCentroid.x;
                    childBBox[1].x = bbox[1].x;
                }
                else {
                    childBBox[0].x = bbox[0].x;
                    childBBox[1].x = nodeCentroid.x;
                }
                // y-axis
                if (extentsCentroid.y > nodeCentroid.y) {
                    childIndex += 2;
                    childBBox[0].y = nodeCentroid.y;
                    childBBox[1].y = bbox[1].y;
                }
                else {
                    childBBox[0].y = bbox[0].y;
                    childBBox[1].y = nodeCentroid.y;
                }
                // z-axis
                if (extentsCentroid.z > nodeCentroid.z) {
                    childIndex += 1;
                    childBBox[0].z = nodeCentroid.z;
                    childBBox[1].z = bbox[1].z;
                }
                else {
                    childBBox[0].z = bbox[0].z;
                    childBBox[1].z = nodeCentroid.z;
                }

                // Create the child node if it doesn't exsit yet and then insert the extents in it
                if (node->child[childIndex] == nullptr)
                    node->child[childIndex] = new OctreeNode;
                insert(node->child[childIndex], extents, childBBox, depth + 1);
            }
        }

        void build(OctreeNode*& node, const BBox& bbox)
        {
            if (node->isLeaf) {
                for (const auto& e: node->nodeExtentsList) {
                    node->nodeExtents.extendBy(*e);
                }
            }
            else {
                for (uint8_t i = 0; i < 8; ++i) {
                        if (node->child[i]) {
                        BBox childBBox;
                        glm::vec3 centroid = bbox.centroid();
                        // x-axis
                        childBBox[0].x = (i & 4) ? centroid.x : bbox[0].x;
                        childBBox[1].x = (i & 4) ? bbox[1].x : centroid.x;
                        // y-axis
                        childBBox[0].y = (i & 2) ? centroid.y : bbox[0].y;
                        childBBox[1].y = (i & 2) ? bbox[1].y : centroid.y;
                        // z-axis
                        childBBox[0].z = (i & 1) ? centroid.z : bbox[0].z;
                        childBBox[1].z = (i & 1) ? bbox[1].z : centroid.z;

                        // Inspect child
                        build(node->child[i], childBBox);

                        // Expand extents with extents of child
                        node->nodeExtents.extendBy(node->child[i]->nodeExtents);
                    }
                }
            }
        }
    };

    std::vector<Extents> extentsList;
    Octree* octree = nullptr;
public:
    BVH(std::list<GameObject *>& m);
    bool intersect(const glm::vec3&, const glm::vec3&, RayHit&) const;
    bool checkBlocked(const glm::vec3&, const glm::vec3&, float d) const;
    ~BVH() { delete octree; }
};
