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
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Application
{
public:
    Physics physics;
    Camera camera;
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

    struct Portal {
        glm::vec3 pos;
        glm::quat rot;
    };

    std::vector<Portal> portals;

    std::vector<Wall> walls;

    void run(const std::vector<std::string> &args);
private:
    void render(float dt);
    void drawScene(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> V);
    void initGeom();
    void makeWall(physx::PxVec3 pos, physx::PxVec3 size, physx::PxQuat rot);
};

extern Application app;