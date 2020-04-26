#include "KDTree.h"
#include "Portal.h"
#include "PortalOutline.h"
#include <glm/glm.hpp>
#include <iostream>

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

bool KDNode::intersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit) {
    float tmin, tmax;
    if (bbox.intersect(orig, dir, tmin, tmax)) {
        return recIntersect(orig, dir, hit, tmin, tmax);
    }
    return false;
}

bool KDNode::recIntersect(const glm::vec3 &orig, const glm::vec3 &dir, RayHit &hit, float tmin, float tmax) {
    if (leaf) {
        bool didHit = false;
        for (Triangle &tri: tris) {
            RayHit tmpHit;
            if (tri.intersect(orig, dir, tmpHit)) {
                tmpHit.obj = tri.obj;
                tmpHit.faceIndex = tri.faceIndex;
                if (didHit) {
                    if (tmpHit < hit) {
                        hit = tmpHit;
                    }
                }
                else {
                    didHit = true;
                    hit = tmpHit;
                }
            }
        }
        return didHit;
    }
    else {
        //float t_split = distanceAlongRayToPlane(ray);
        float tsplit = (plane.pos - orig[plane.axis]) * (dir[plane.axis] == 0 ? INFINITY : 1.f / dir[plane.axis]);

        // near is the side containing the origin of the ray
        KDNode *near, *far;
        if (orig[plane.axis] < plane.pos) {
            near = left.get();
            far = right.get();
        } else {
            near = right.get();
            far = left.get();
        }

        if (tsplit > tmax || tsplit < 0) {
            return near->recIntersect(orig, dir, hit, tmin, tmax);
        }
        else if (tsplit < tmin) {
            return far->recIntersect(orig, dir, hit, tmin, tmax);
        }
        else {
            if (near->recIntersect(orig, dir, hit, tmin, tsplit) && hit.d < tsplit)
                return true;
            return far->recIntersect(orig, dir, hit, tsplit, tmax);
        }
    }
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool BBox::intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &tmin, float &tmax, float d) {
    tmin = (bbMin.x - orig.x) / dir.x;
    tmax = (bbMax.x - orig.x) / dir.x;

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

BBox Triangle::getBounds() const {
    BBox box;
    box.bbMin = box.bbMax = verts[0];
    for (const vec3 &vert : verts) {
        for (int i = 0; i < 3; i++) {
            box.bbMin[i] = std::min(box.bbMin[i], vert[i]);
            box.bbMax[i] = std::max(box.bbMax[i], vert[i]);
        }
    }
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

// build list of triangles, then make recursive call to build tree
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

    BBox box;
    box.bbMin = box.bbMax = tris[0].verts[0];
    for (const Triangle &tri : tris) {
        for (const vec3 &vert : tri.verts) {
            for (int i = 0; i < 3; i++) {
                box.bbMin[i] = std::min(box.bbMin[i], vert[i]);
                box.bbMax[i] = std::max(box.bbMax[i], vert[i]);
            }
        }
    }

    SplitPlane dummyPlane;
    dummyPlane.axis = -1;

    return recBuild(tris, box, 0, dummyPlane);
}

std::unique_ptr<KDNode> KDNode::recBuild(const std::vector<Triangle> &tris, const BBox &V, int depth, const SplitPlane &prevPlane) {
    SplitPlane p;
    float Cp;
    PlaneSide pside;
    findPlane(tris, V, depth, p, Cp, pside);

    if (isDone(tris.size(), Cp) || p == prevPlane) {
        // Leaf node
        unique_ptr<KDNode> leafnode = make_unique<KDNode>();
        leafnode->tris = tris;
        leafnode->bbox = V;
        leafnode->leaf = true;
        return leafnode;
    }

    BBox VL, VR;
    splitBox(V, p, VL, VR);
    std::vector<Triangle> TL, TR;
    sortTriangles(tris, p, pside, TL, TR);
    // Inner node
    unique_ptr<KDNode> innerNode = make_unique<KDNode>();
    innerNode->plane = p;
    innerNode->bbox = V;
    innerNode->leaf = false;
    innerNode->left = recBuild(TL, VL, depth+1, p);
    innerNode->right = recBuild(TR, VR, depth+1, p);
    return innerNode;
}

// get primitives's clipped bounding box
BBox clipTriangleToBox(const Triangle &t, const BBox &V) {
    BBox b = t.getBounds();
    for(int k=0; k<3; k++) {
        if(V.bbMin[k] > b.bbMin[k])
            b.bbMin[k] = V.bbMin[k];
        if(V.bbMax[k] < b.bbMax[k])
            b.bbMax[k] = V.bbMax[k];
    }
    return b;
}


struct Event {
    typedef enum { endingOnPlane=0, lyingOnPlane=1, startingOnPlane=2  } EventType;
    const Triangle *triangle;
    SplitPlane splitPlane;
    EventType type;

    Event(const Triangle *tri, int k, float ee0, EventType type) : triangle(tri), type(type), splitPlane(SplitPlane(k, ee0)){}

    inline bool operator<(const Event& e) const {
        return((splitPlane.pos < e.splitPlane.pos) || (splitPlane.pos == e.splitPlane.pos && type < e.type));
    }
};

// best spliting plane using SAH heuristic
void KDNode::findPlane(const std::vector<Triangle> &T, const BBox &V, int depth, SplitPlane &pEst, float &cEst, PlaneSide &psideEst) {
    // static int count = 0;
    cEst = INFINITY;
    for(int k=0; k<3; ++k) {
        std::vector<Event> events;
        events.reserve(T.size()*2);
        for (const Triangle &t : T) {
            BBox B = clipTriangleToBox(t, V);
            if(B.isPlanar()) {
                events.push_back(Event(&t, k, B.bbMin[k], Event::lyingOnPlane));
            } else {
                events.push_back(Event(&t, k, B.bbMin[k], Event::startingOnPlane));
                events.push_back(Event(&t, k, B.bbMax[k], Event::endingOnPlane));
            }
        }
        sort(events.begin(), events.end());
        int NL = 0, NP = 0, NR = T.size();
        for(std::vector<Event>::size_type Ei = 0; Ei < events.size(); ++Ei) {
            const SplitPlane& p = events[Ei].splitPlane;
            int pLyingOnPlane = 0, pStartingOnPlane = 0, pEndingOnPlane = 0;
            while(Ei < events.size() && events[Ei].splitPlane.pos == p.pos && events[Ei].type == Event::endingOnPlane) {
                ++pEndingOnPlane;
                Ei++;
            }
            while(Ei < events.size() && events[Ei].splitPlane.pos == p.pos && events[Ei].type == Event::lyingOnPlane) {
                ++pLyingOnPlane;
                Ei++;
            }
            while(Ei < events.size() && events[Ei].splitPlane.pos == p.pos && events[Ei].type == Event::startingOnPlane) {
                ++pStartingOnPlane;
                Ei++;
            }
            NP = pLyingOnPlane;
            NR -= pLyingOnPlane;
            NR -= pEndingOnPlane;
            float C;
            PlaneSide pside = UNKNOWN;
            SAH(p, V, NL, NR, NP, C, pside);
            if(C < cEst) {
                cEst = C;
                pEst = p;
                psideEst = pside;
            }
            NL += pStartingOnPlane;
            NL += pLyingOnPlane;
            NP = 0;
        }
    }
}

bool KDNode::checkBlocked(const glm::vec3 &orig, const glm::vec3 &dir, float d) {
    RayHit hit;
    return intersect(orig, dir, hit) && hit.d < d;
}

void KDNode::sortTriangles(const std::vector<Triangle> &T, const SplitPlane &p, const PlaneSide &pside, std::vector<Triangle> &TL, std::vector<Triangle> &TR) {
    for(const Triangle &t : T) {
        BBox tbox = t.getBounds();
        if(tbox.bbMin[p.axis] == p.pos && tbox.bbMax[p.axis] == p.pos) {
            if(pside == LEFT)
                TL.push_back(t);
            else if(pside == RIGHT)
                TR.push_back(t);
            else
                std::cout << "ERROR WHILE SORTING TRIANLGES" << std::endl;
        } else {
            if(tbox.bbMin[p.axis] < p.pos)
                TL.push_back(t);
            if(tbox.bbMax[p.axis] > p.pos)
                TR.push_back(t);
        }
    }
}

// surface area of a volume V
float surfaceArea(const BBox& V) {
    return 2*V.dx()*V.dy() + 2*V.dx()*V.dz() + 2*V.dy()*V.dz();
}

// Probability of hitting volume Vsub, given volume V was hit
float prob_hit(const BBox& Vsub, const BBox& V){
    return surfaceArea(Vsub) / surfaceArea(V);
}

// bias for the cost function s.t. it is reduced if NL or NR becomes zero
float lambda(int NL, int NR, float PL, float PR) {
    if((NL == 0 || NR == 0) &&
       !(PL == 1 || PR == 1) // NOT IN PAPER
       )
        return 0.8f;
    return 1.0f;
}

inline float cost(float PL, float PR, int NL, int NR) {
    return(lambda(NL, NR, PL, PR) * (COST_TRAVERSE + COST_INTERSECT * (PL * NL + PR * NR)));
}

void KDNode::SAH(const SplitPlane &p, const BBox &V, int NL, int NR, int NP, float &CP, PlaneSide& pside) {
    CP = INFINITY;
    BBox VL, VR;
    splitBox(V, p, VL, VR);
    float PL, PR;
    PL = prob_hit(VL, V);
    PR = prob_hit(VR, V);
    if(PL == 0 || PR == 0) // NOT IN PAPER
        return;
    if(V.d(p.axis) == 0) // NOT IN PAPER
        return;
    float CPL, CPR;
    CPL = cost(PL, PR, NL + NP, NR);
    CPR = cost(PL, PR, NL, NP + NR );
    if(CPL < CPR) {
        CP = CPL;
        pside = LEFT;
    } else {
        CP = CPR;
        pside = RIGHT;
    }
}

void KDNode::splitBox(const BBox &V, const SplitPlane &p, BBox &VL, BBox &VR) {
    VL = V;
    VR = V;
    VL.bbMax[p.axis] = p.pos;
    VR.bbMin[p.axis] = p.pos;
}

// criterion for stopping subdividing a tree node
bool KDNode::isDone(int N, float minCv) {
    // cerr << "terminate: minCv=" << minCv << ", KI*N=" << KI*N << endl;
    return(minCv > COST_INTERSECT*N);
}
