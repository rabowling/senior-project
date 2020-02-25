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
#include "Box.h"
#include "Button.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

#define MAX_PORTALS 256

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

    std::vector<Portal> portals;
    std::vector<Box> boxes;
    std::vector<Button> buttons;
    std::vector<Wall> walls;

    void run(const std::vector<std::string> &args);
private:
    void render(float dt);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void loadLevel(std::string levelFile);
};

extern Application app;