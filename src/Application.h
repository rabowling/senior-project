#pragma once

#include <PxPhysicsAPI.h>
#include "Shape.h"
#include "Physics.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "WindowManager.h"
#include "Player.h"
#include "Controls.h"
#include "Wall.h"
#include "Portal.h"
#include "Box.h"
#include "Button.h"
#include "PortalOutline.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

#define MAX_PORTALS 256

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

struct Light {
    glm::vec3 position;
    glm::vec3 intensity;
};

class Application
{
public:
    enum RenderMode { RENDER_OPENGL, RENDER_RAYTRACE };
    Physics physics;
    ShaderManager shaderManager;
    TextureManager textureManager;
    ModelManager modelManager;
    MaterialManager materialManager;
    WindowManager windowManager;
    Player player;
    Controls controls;

    int stepCount = 0;
    
    float physicsStep;
    float deltaTime;

    std::list<Portal> portals;
    std::list<Box> boxes;
    std::list<Button> buttons;
    std::list<Wall> walls;
    std::list<GameObject *> gameObjects;

    unsigned int depthCubemap;
    unsigned int depthMapFBO;
    std::vector<Light> lights;
    glm::vec3 lightPos;
    float near = 1.0f;
    float far = 100.0f;
    float lightSpeed = 5.0f;
    bool renderingCubemap = false;
    bool renderingFP = false;

    void run(Controls::InputMode inputMode, std::string recordFilename, RenderMode renderMode);
private:
    void render(float dt);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void loadLevel(std::string levelFile);
    void renderToCubemap(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera, const bool isCubemap);
    void initCubemap();
};

extern Application app;