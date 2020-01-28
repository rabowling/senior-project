#pragma once

#include <PxPhysicsAPI.h>
#include "Shape.h"
#include "Physics.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "WindowManager.h"
#include "Player.h"
#include "Controls.h"
#include <string>

class Application
{
public:
    Physics physics;
    Camera camera;
    ShaderManager shaderManager;
    WindowManager windowManager;
    Player player;
    Controls controls;
    
    float physicsStep;
    float deltaTime;

    physx::PxRigidDynamic *gBox = NULL;
    physx::PxRigidDynamic *gBox2 = NULL;
    physx::PxRigidStatic *gGroundPlane = NULL;

    std::shared_ptr<Shape> boxShape;
    std::shared_ptr<Shape> planeShape;

    void run();
    void render(float dt);
    void initGeom(std::string resourceDirectory);
};

extern Application app;