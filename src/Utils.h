#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>

std::vector<std::string> listDir(std::string dir);
physx::PxVec3 glm2px(glm::vec3 v);
physx::PxExtendedVec3 glm2pxex(glm::vec3 v);
glm::vec3 px2glm(physx::PxVec3 v);
glm::vec3 px2glm(physx::PxExtendedVec3 v);
glm::quat px2glm(physx::PxQuat q);