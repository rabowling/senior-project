#include "Utils.h"
#include <string>
#include <vector>
#include <dirent.h>
#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>

using namespace physx;

std::vector<std::string> listDir(std::string dir) {
    struct dirent *entry = nullptr;
    DIR *dp = opendir(dir.c_str());
    std::vector<std::string> dirs;

    while ((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            dirs.push_back(std::string(entry->d_name));
        }
    }

    closedir(dp);
    return dirs;
}

physx::PxVec3 glm2px(glm::vec3 v) {
    return PxVec3(v.x, v.y, v.z);
}

glm::vec3 px2glm(physx::PxExtendedVec3 v) {
    return glm::vec3(v.x, v.y, v.z);
}
