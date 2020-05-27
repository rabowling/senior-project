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
#include "LightSwitch.h"
#include "MiscItem.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

#define MAX_PORTALS 256

/* 2 player controlled portals, 2 world portals. */
#define NUM_PORTALS 4
#define ORANGE_PORTAL 0
#define BLUE_PORTAL 1
#define WORLD_PORTAL_1 2
#define WORLD_PORTAL_2 3

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

#define INNER_CUTOFF glm::cos(glm::radians(30.0f))
#define OUTER_CUTOFF glm::cos(glm::radians(45.0f))
#define MIN_LIGHT_DIST 5
#define MAX_LIGHT_DIST 30

struct Light {
    glm::vec3 position;
    glm::vec3 intensity;
    int id;
};

/* Used for real-time portal lighting */
struct PortalLight {
    glm::vec3 position;
    glm::vec3 direction;
    float intensity;
    Portal *portal;
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

    int width, height;

    int stepCount = 0;
    
    float physicsStep;
    float deltaTime;

    std::list<Portal> portals;
    std::list<Box> boxes;
    std::list<Button> buttons;
    std::list<Wall> walls;
    std::list<LightSwitch> switches;
    std::list<MiscItem> miscItems;
    std::list<Door> doors;
    std::list<GameObject *> gameObjects;

    unsigned int depthCubemap;
    unsigned int depthMapFBO[NUM_PORTALS + 1];
    unsigned int depthMaps[NUM_PORTALS];
    unsigned int units[NUM_PORTALS] = {5, 6, 7, 8};
    std::vector<Light> lights;
    glm::vec3 lightPos;
    Light currentLight;
    PortalLight portalLights[NUM_PORTALS];
    //glm::mat4 LP = glm::ortho(-100.0, 100.0, -100.0, 100.0, 0.1, 100.0);
    glm::mat4 LP;

    glm::mat4 orthoProjection;

    float near = 1.0f;
    float far = 100.0f;
    float lightSpeed = 5.0f;
    bool renderingCubemap = false;
    bool renderingFP = false;
    int numSamplesShadows = 1;

    void run(std::string levelFilename, Controls::InputMode inputMode, std::string recordFilename, RenderMode renderMode);
private:
    void update(float dt);
    void render(float dt);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void loadLevel(std::string levelFile);
    void renderToCubemap(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera);
    void renderToDepthmap(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera, std::string shader);
    void drawScene(const glm::mat4 &P, const glm::mat4 &V, const Camera &camera, const bool isCubemap);
    void initCubemap();
    void initDepthmaps();
    void updatePortalLights();
};

extern Application app;