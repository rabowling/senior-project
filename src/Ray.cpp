#include "Ray.h"
#include <cmath>
#include "Portal.h"
#include "PortalOutline.h"

bool RayHit::operator<(const RayHit &rhs) {
    if (std::abs(d - rhs.d) < 0.0001) {
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