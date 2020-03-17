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
#define NUM_PORTALS 4

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

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
    glm::vec3 lightPos;
    float near = 1.0f;
    float far = 100.0f;
    float lightSpeed = 5.0f;
    bool renderingCubemap = false;

    unsigned int portalLightMapFBOs[NUM_PORTALS];
    unsigned int portalLightDepthMaps[NUM_PORTALS];
    bool renderingLightMaps = false;
    glm::mat4 lightProjection, lightView;

    void run(Controls::InputMode inputMode, std::string recordFilename, RenderMode renderMode);
private:
    void render(float dt);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera, std::string shader, std::string wallShader, unsigned int depthmap);
    void loadLevel(std::string levelFile);
    void renderToCubemap(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void initCubemap();
    void initLightMaps();
    void renderLightMaps(const glm::mat4 &P, const glm::mat4 &V, MatrixStack &M, const Camera &camera);
    void renderPortals(const glm::mat4 &P, const glm::mat4 &V, MatrixStack M, const Camera &camera, bool portalLight);
};

extern Application app;