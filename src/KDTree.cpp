#include "KDTree.h"
#include "Portal.h"
#include "PortalOutline.h"
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

#define EPSILON 0.00001

bool RayHit::operator<(const RayHit &rhs) {
    if (abs(d - rhs.d) < 0.0001) {
        if (dynamic_cast<Portal *>(obj)) {
            return true;
        }
        else if (dynamic_cast<Portal *>(rhs.obj)) {
            return false;
        }
        else if (dynamic_cast<PortalOutline *>(obj)) {
            return true;
        }
        else if (dynamic_cast<PortalOutline *>(rhs.obj)) {
            return false;
        }
    }
    return d < rhs.d;
}

BBox::BBox() : bbMin(0), bbMax(0)
{

}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool BBox::intersect(const glm::vec3 &orig, const glm::vec3 &dir, float d) {
    float tmin = (bbMin.x - orig.x) / dir.x;
    float tmax = (bbMax.x - orig.x) / dir.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (bbMin.y - orig.y) / dir.y;
    float tymax = (bbMax.y - orig.y) / dir.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (bbMin.z - orig.z) / dir.z;
    float tzmax = (bbMax.z - orig.z) / dir.z;

    if (tzmin > tzmax) swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (d > 0) {
        if (tzmin > tmin)
            tmin = tzmin;

        if (tzmax < tmax)
            tmax = tzmax;

        return tmin < d;
    }

    return true;
}

// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool Triangle::intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit) const {
    vec3 edge1 = verts[1] - verts[0];
    vec3 edge2 = verts[2] - verts[0];
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);

    if (det < EPSILON) {
        return false;
    }

    vec3 tvec = orig - verts[0];
    hit.u = dot(tvec, pvec);
    if (hit.u < 0 || hit.u > det) {
        return false;
    }

    vec3 qvec = cross(tvec, edge1);
    hit.v = dot(dir, qvec);
    if (hit.v < 0 || hit.u + hit.v > det) {
        return false;
    }

    hit.d = dot(edge2, qvec);
    float inv_det = 1 / det;
    hit.d *= inv_det;
    hit.u *= inv_det;
    hit.v *= inv_det;

    return hit.d > -EPSILON;
}

std::unique_ptr<KDNode> KDNode::build(const std::list<GameObject *> &gameObjects) {
    vector<Triangle> tris;

    for (GameObject *obj : gameObjects) {
        Shape *model = obj->getModel();
        mat4 transform = obj->getTransform();
        if (obj->posBufCache.size() != model->posBuf.size()) {
            obj->posBufCache.resize(model->posBuf.size());
        }

        // loop over each face
        for (int fIdx = 0; fIdx < model->eleBuf.size() / 3; fIdx++) {
            Triangle tri;
            tri.faceIndex = fIdx;
            tri.obj = obj;

            for (int vNum = 0; vNum < 3; vNum++) {
                // get transformed vertex coordinates
                unsigned int vIdx = model->eleBuf[fIdx*3+vNum];
                for (int i = 0; i < 3; i++) {
                    tri.verts[vNum][i] = model->posBuf[vIdx*3+i];
                }
                tri.verts[vNum] = vec3(transform * vec4(tri.verts[vNum], 1));

                // cache transformed vertex coordinates
                for (int i = 0; i < 3; i++) {
                    obj->posBufCache[vIdx*3+i] = tri.verts[vNum][i];
                }
            }
            tris.push_back(tri);
        }
    }

    return build(tris, 0);
}

// https://blog.frogslayer.com/kd-trees-for-faster-ray-tracing-with-triangles/
std::unique_ptr<KDNode> KDNode::build(const std::vector<Triangle> &tris, int depth) {
    unique_ptr<KDNode> node = make_unique<KDNode>();
    node->tris = tris;
    
    if (tris.size() == 0) {
        return node;
    }

    node->bbox.bbMin = glm::min(tris[0].verts[0], glm::min(tris[0].verts[1], tris[0].verts[2]));
    node->bbox.bbMax = glm::max(tris[0].verts[0], glm::max(tris[0].verts[1], tris[0].verts[2]));

    if (tris.size() == 1) {
        node->left = make_unique<KDNode>();
        node->right = make_unique<KDNode>();
        return node;
    }

    for (int i = 1; i < tris.size(); i++) {
        node->bbox.bbMin = glm::min(glm::min(node->bbox.bbMin, tris[i].verts[0]), glm::min(tris[i].verts[1], tris[i].verts[2]));
        node->bbox.bbMax = glm::max(glm::max(node->bbox.bbMax, tris[i].verts[0]), glm::max(tris[i].verts[1], tris[i].verts[2]));
    }

    vec3 midpoint(0);
    for (const Triangle &tri : tris) {
        midpoint += tri.verts[0] + tri.verts[1] + tri.verts[2];
    }
    midpoint /= 3.0f * tris.size();

    vector<Triangle> leftTris;
    vector<Triangle> rightTris;
    vec3 bboxSize = node->bbox.bbMax - node->bbox.bbMin;
    float longestAxis = std::max({bboxSize.x, bboxSize.y, bboxSize.z});
    for (const Triangle &tri : tris) {
        vec3 triMidpoint = (tri.verts[0] + tri.verts[1] + tri.verts[2]) / 3.0f;
        for (int i = 0; i < 3; i++) {
            if (longestAxis == bboxSize[i]) {
                midpoint[i] >= triMidpoint[i] ? rightTris.push_back(tri) : leftTris.push_back(tri);
                break;
            }
        }
    }

    if (leftTris.size() == 0 && rightTris.size() > 0) {
        leftTris = rightTris;
    }
    else if (rightTris.size() == 0 && leftTris.size() > 0) {
        rightTris = leftTris;
    }

    int matches = 0;
    for (const Triangle &leftTri : leftTris) {
        for (const Triangle &rightTri : rightTris) {
            if (leftTri.obj == rightTri.obj && leftTri.faceIndex == rightTri.faceIndex) {
                matches++;
            }
        }
    }

    if ((float) matches / leftTris.size() < 0.5 && (float) matches / rightTris.size() < 0.5) {
        node->left = build(leftTris, depth + 1);
        node->right = build(rightTris, depth + 1);
    }
    else {
        node->left = make_unique<KDNode>();
        node->right = make_unique<KDNode>();
    }

    return node;
}

bool KDNode::intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &closestHit) {
    if (!bbox.intersect(orig, dir)) {
        return false;
    }

    if (left->tris.size() > 0 || right->tris.size() > 0) {
        RayHit leftHit, rightHit;
        bool didHitLeft = left->intersect(orig, dir, leftHit);
        bool didHitRight = right->intersect(orig, dir, rightHit);
        if (didHitLeft && didHitRight) {
            closestHit = rightHit < leftHit ? rightHit : leftHit;
        }
        else if (didHitLeft) {
            closestHit = leftHit;
        }
        else if (didHitRight) {
            closestHit = rightHit;
        }
        return didHitLeft || didHitRight;
    }
    else {
        bool didHit = false;
        RayHit hit;
        for (const Triangle &tri : tris) {
            if (tri.intersect(orig, dir, hit)) {
                hit.obj = tri.obj;
                hit.faceIndex = tri.faceIndex;

                if (didHit) {
                    if (hit < closestHit) {
                        closestHit = hit;
                    }
                }
                else {
                    closestHit = hit;
                    didHit = true;
                }
            }
        }

        return didHit;
    }
}

bool KDNode::checkBlocked(const glm::vec3 &orig, const glm::vec3 &dir, float d) {
    if (!bbox.intersect(orig, dir, d)) {
        return false;
    }

    if (left->tris.size() > 0 || right->tris.size() > 0) {
        return left->checkBlocked(orig, dir, d) || right->checkBlocked(orig, dir, d);
    }
    else {
        RayHit hit;
        for (const Triangle &tri : tris) {
            if (tri.intersect(orig, dir, hit)) {
                if (hit.d < d) {
                    return true;
                }
            }
        }

        return false;
    }
}

