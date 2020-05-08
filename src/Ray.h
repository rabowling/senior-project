#pragma once

class GameObject;
struct RayHit {
    float d;
    float u, v;
    unsigned int faceIndex;
    GameObject *obj;
    bool operator<(const RayHit &rhs);
    bool operator>(const RayHit &rhs) { return !operator<(rhs); }
};