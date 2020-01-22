#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>

std::vector<std::string> listDir(std::string dir);
physx::PxVec3 glm2px(glm::vec3 v);
