#pragma once

#include <PxPhysicsAPI.h>
#include "Shape.h"
#include "Physics.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "WindowManager.h"
#include "Player.h"
#include "Controls.h"
#include "Wall.h"
#include <string>
#include <vector>

class Application
{
public:
    Physics physics;
    Camera camera;
    ShaderManager shaderManager;
    TextureManager textureManager;
    WindowManager windowManager;
    Player player;
    Controls controls;

    long stepCount = 0;
    
    float physicsStep;
    float deltaTime;

    physx::PxRigidDynamic *gBox = NULL;
    physx::PxRigidDynamic *gBox2 = NULL;
    physx::PxRigidStatic *gGroundPlane = NULL;
    physx::PxRigidStatic *gButton = NULL;
    bool buttonPressed = false;

    std::shared_ptr<Shape> boxShape;
    std::shared_ptr<Shape> planeShape;
    std::shared_ptr<Shape> cylinderShape;

    std::vector<Wall> walls;

    void run();
private:
    void render(float dt);
    void initGeom(std::string resourceDirectory);
    void makeWall(physx::PxVec3 pos, physx::PxVec3 size, physx::PxQuat rot);
};

extern Application app;