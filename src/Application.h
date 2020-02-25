#pragma once

#include <PxPhysicsAPI.h>
#include "Shape.h"
#include "Physics.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WindowManager.h"
#include "Player.h"
#include "Controls.h"
#include "Wall.h"
#include "Portal.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

#define MAX_PORTALS 256

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

class Application
{
public:
    Physics physics;
    ShaderManager shaderManager;
    TextureManager textureManager;
    ModelManager modelManager;
    WindowManager windowManager;
    Player player;
    Controls controls;

    int stepCount = 0;
    
    float physicsStep;
    float deltaTime;

    physx::PxRigidDynamic *gBox = NULL;
    physx::PxRigidDynamic *gBox2 = NULL;
    physx::PxRigidStatic *gGroundPlane = NULL;
    physx::PxRigidStatic *gButton = NULL;
    bool buttonPressed = false;

    std::vector<Portal> portals;

    std::vector<Wall> walls;

    unsigned int depthCubemap;
    unsigned int depthMapFBO;
    glm::vec3 lightPos;
    float near = 1.0f;
    float far = 100.0f;

    void run(const std::vector<std::string> &args);
private:
    void render(float dt);
    void renderToCubemap(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera, const bool isCubemap);
    void initGeom();
    void initCubemap();
    void makeWall(physx::PxVec3 pos, physx::PxVec3 size, physx::PxQuat rot);
};

extern Application app;