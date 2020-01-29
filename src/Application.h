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

    void run();
private:
    void render(float dt);
    void initGeom(std::string resourceDirectory);
};

extern Application app;