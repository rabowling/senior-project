#include "Application.h"

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "Shape.h"
#include "GLSL.h"
#include "Raytrace.h"

using namespace physx;
using namespace std;
using namespace glm;

#define FPS 1.0f / 60.0f

Application app;

void Application::run(Controls::InputMode inputMode, std::string recordFilename, RenderMode renderMode) {
    windowManager.init(1280, 720);
    physics.init();
    player.init();
    controls.init(inputMode, recordFilename);

    loadLevel("../resources/levels/level1.txt");
    initCubemap();
    shaderManager.loadShaders("../resources/shaders");
    textureManager.loadTextures("../resources/textures");
    modelManager.loadModels("../resources/models");
    materialManager.loadMaterials();

    lightPos = vec3(30, 8, 30);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    while (!glfwWindowShouldClose(windowManager.getHandle())) {
        controls.update();
        player.update(1.0f / 60.0f);
        physics.getScene()->simulate(1.0f / 60.0f);
        physics.getScene()->fetchResults(true);
        render(1.0f / 60.0f);
        glfwSwapBuffers(windowManager.getHandle());
        glfwPollEvents();

        if (renderMode == RENDER_RAYTRACE) {
            string numString = to_string(stepCount);
            numString = string(5 - numString.length(), '0') + numString;
            renderRT(1920, 1080, "render/frame" + numString + ".ppm");
            if (controls.playbackFinished()) {
                break;
            }
        }
        stepCount++;
    }

    windowManager.shutdown();
}

void Application::render(float dt) {
    int width, height;
    glfwGetFramebufferSize(windowManager.getHandle(), &width, &height);

    float aspect = width / (float) height;
    mat4 V = player.camera.getLookAt();
    mat4 P = glm::perspective(45.0f, aspect, 0.1f, 100.0f);

    if (controls.isHeld(Controls::LIGHT_FORWARD)) {
        lightPos += vec3(0,0,-1) * dt * lightSpeed;
    }
    if (controls.isHeld(Controls::LIGHT_BACKWARD)) {
        lightPos += vec3(0,0,1) * dt * lightSpeed;
    }
    if (controls.isHeld(Controls::LIGHT_LEFT)) {
        lightPos += vec3(-1,0,0) * dt * lightSpeed;
    }
    if (controls.isHeld(Controls::LIGHT_RIGHT)) {
        lightPos += vec3(1,0,0) * dt * lightSpeed;
    }
    if (controls.isHeld(Controls::LIGHT_UP)) {
        lightPos += vec3(0,1,0) * dt * lightSpeed;
    }
    if (controls.isHeld(Controls::LIGHT_DOWN)) {
        lightPos += vec3(0,-1,0) * dt * lightSpeed;
    }

    // Create MVP matrices
    MatrixStack M;

    renderToCubemap(P, V, player.camera);
    renderLightMaps(P, V, M, player.camera);

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear framebuffer.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.12f, .34f, .56f, 1.0f);
    glStencilMask(0x00);

    shaderManager.bind("portal");
    drawScene(P, V, player.camera, "tex", "wall", depthCubemap);
    renderPortals(P, V, M, player.camera, false);
    renderPortals(P, V, M, player.camera, true);
}

void Application::renderPortals(const mat4 &P, const mat4 &V, MatrixStack M, const Camera &camera, bool portalLight) {

    // Draw geometry of portals to stencil buffer
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, value_ptr(P));
    glStencilMask(0xFF);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    int i = 1;
    for (Portal &portal : portals) {
        glStencilFunc(GL_ALWAYS, i++, 0xFF);
        portal.draw(M);
    }

    // Render portal outline
    glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);

    i = 1;
    for (Portal &portal : portals) {
        glStencilFunc(GL_NOTEQUAL, i++, 0xFF);
        if (portal.hasOutline) {
            portal.outline->draw(M);
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    //glEnable(GL_DEPTH_TEST);
    // Render scene through portals
    i = 1;
    for (Portal &portal : portals) {
        int index = i++;
        glStencilFunc(GL_EQUAL, index, 0xFF);
        glClear(GL_DEPTH_BUFFER_BIT);
        Portal *linkedPortal = portal.linkedPortal;
        linkedPortal->updateCamera(player.camera);
        mat4 portalV = linkedPortal->camera.getLookAt();
        mat4 portalP = linkedPortal->modifyProjectionMatrix(P, portalV);
        if (portalLight) {
            drawScene(portalP, portalV, linkedPortal->camera, "portallight", "walllight", portalLightDepthMaps[index-1]);
        } else {
            drawScene(portalP, portalV, linkedPortal->camera, "tex", "wall", depthCubemap);
        }

        if (!renderingLightMaps) {
            shaderManager.bind("portal");
            glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(portalP));
            glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(portalV));
        }
        glEnable(GL_POLYGON_OFFSET_FILL);
        for (Portal &portal : portals) {
            portal.draw(M);
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void Application::renderToCubemap(const mat4 &P, const mat4 &V, const Camera &camera) {
    app.renderingCubemap = true;
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    mat4 shadowProj = perspective(radians(90.0f), aspect, near, far);

    vector<mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    shaderManager.bind("cubemap");

    for (unsigned int i = 0; i < 6; i++) {
        glUniformMatrix4fv(shaderManager.getUniform("shadowMatrices[" + to_string(i) + "]"), 1, GL_FALSE, value_ptr(shadowTransforms[i]));
    }
    glUniform1f(shaderManager.getUniform("farPlane"), far);
    glUniform3fv(shaderManager.getUniform("lightPos"), 1, value_ptr(lightPos));

    glCullFace(GL_FRONT);
    drawScene(P, V, camera, "", "", 1);
    glCullFace(GL_BACK);
    app.renderingCubemap = false;
}

void Application::drawScene(const mat4 &P, const mat4 &V, const Camera &camera, string shader, string wallShader, unsigned int depthmap) {
    MatrixStack M;

    mat4 LS = lightProjection * lightView;

    if (!renderingCubemap && !renderingLightMaps) {
        shaderManager.bind(shader);
        glUniformMatrix4fv(shaderManager.getUniform("lightingMatrix"), 1, GL_FALSE, value_ptr(LS));
        glUniform3fv(shaderManager.getUniform("lightPos"), 1, value_ptr(lightPos));
        glUniform1f(shaderManager.getUniform("farPlane"), far);
        glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
        glActiveTexture(GL_TEXTURE1);
        if (shader == "tex") {
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap);
        } else {
            glBindTexture(GL_TEXTURE_2D, depthmap);
        }
    }
    for (Box &box : boxes) {
        box.draw(M);
    }

    if (!renderingCubemap && !renderingLightMaps) {
        glUniform3fv(shaderManager.getUniform("lightPos"), 1, value_ptr(lightPos));
        glUniform1f(shaderManager.getUniform("farPlane"), far);
        glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
        glActiveTexture(GL_TEXTURE1);
        if (shader == "tex") {
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap);
        } else {
            glBindTexture(GL_TEXTURE_2D, depthmap);
        }
    }
    for (Button &button : buttons) {
        button.draw(M);
    }

    if (!renderingCubemap && !renderingLightMaps) {
        // Set up wall shader colors here
        shaderManager.bind(wallShader);
        glUniform3fv(shaderManager.getUniform("lightPos"), 1, value_ptr(lightPos));
	    glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform1f(shaderManager.getUniform("farPlane"), far);
        glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
        glActiveTexture(GL_TEXTURE1);
        if (shader == "tex") {
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap);
        } else {

            glBindTexture(GL_TEXTURE_2D, depthmap);
        }
    }
    for (Wall &wall : walls) {
        wall.draw(M);
    }
}

void Application::renderLightMaps(const glm::mat4 &P, const glm::mat4 &V, MatrixStack &M, const Camera &camera) {
    int i = 0;

    app.renderingLightMaps = true;
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    for (Portal portal : portals) {
        glBindFramebuffer(GL_FRAMEBUFFER, portalLightMapFBOs[i]);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        lightProjection = ortho(-100.0, 100.0, -100.0, 100.0, 0.1, 100.0);
        lightView = lookAt(lightPos, portal.position, vec3(0,1,0));

        shaderManager.bind("lightmap");
        glUniformMatrix4fv(shaderManager.getUniform("lightProjection"), 1, GL_FALSE, value_ptr(lightProjection));
        glUniformMatrix4fv(shaderManager.getUniform("lightView"), 1, GL_FALSE, value_ptr(lightView));
        renderPortals(P, V, M, camera, true);
        glCullFace(GL_BACK);
    }

    app.renderingLightMaps = false;
}

void Application::initLightMaps() {
    for (int i = 0; i < NUM_PORTALS; i++) {
        glGenFramebuffers(1, &portalLightMapFBOs[i]);
        glGenTextures(1, &portalLightDepthMaps[i]);
        glBindTexture(GL_TEXTURE_2D, portalLightDepthMaps[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindFramebuffer(GL_FRAMEBUFFER, portalLightMapFBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, portalLightDepthMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Application::initCubemap() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::loadLevel(string levelFile) {
    ifstream in;
    in.open("../resources/levels/level1.txt");
    string line;
    map<int, Portal *> portalIdMap;
    while (getline(in, line)) {
        istringstream iss(line);
        string type;
        getline(iss, type, ' ');
        if (type == "wall") {
            float data[10];
            for (int i = 0; i < 10; i++) {
                iss >> data[i];
                iss.ignore();
            }
            PxVec3 pos(data[0], data[1], data[2]);
            PxVec3 scale(data[3], data[4], data[5]);
            PxQuat rot(data[6], data[7], data[8], data[9]);
            walls.push_back(Wall());
            walls.rbegin()->init(pos, scale, rot);
        }
        else if (type == "player") {
            float data[3];
            for (int i = 0; i < 3; i++) {
                iss >> data[i];
                iss.ignore();
            }
            player.setPosition(data[0], data[1], data[2]);
        }
        else if (type == "portal") {
            int intData[2];
            float floatData[10];
            for (int i = 0; i < 2; i++) {
                iss >> intData[i];
                iss.ignore();
            }
            for (int i = 0; i < 10; i++) {
                iss >> floatData[i];
                iss.ignore();
            }
            vec3 pos = glm::vec3(floatData[0], floatData[1], floatData[2]);
            vec3 scale = glm::vec3(floatData[3], floatData[4], floatData[5]);
            quat rot = glm::quat(floatData[6], floatData[7], floatData[8], floatData[9]);
            int portalId = intData[0];
            int linkedPortalId = intData[1];
            portals.push_back(Portal(pos, scale, rot, "world_portal"));
            Portal *portal = &portals.back();
            portalIdMap[portalId] = portal;
            portal->setPosition(pos, rot);

            // link portals
            if (portalIdMap.find(linkedPortalId) != portalIdMap.end()) {
                portal->linkPortal(portalIdMap[linkedPortalId]);
            }
        }
        else if (type == "box") {
            float data[10];
            for (int i = 0; i < 10; i++) {
                iss >> data[i];
                iss.ignore();
            }
            PxVec3 pos(data[0], data[1], data[2]);
            PxVec3 scale(data[3], data[4], data[5]);
            PxQuat rot(data[6], data[7], data[8], data[9]);
            boxes.push_back(Box());
            boxes.rbegin()->init(pos, scale, rot);
        }
        else if (type == "button") {
            float data[3];
            for (int i = 0; i < 3; i++) {
                iss >> data[i];
                iss.ignore();
            }
            PxVec3 pos(data[0], data[1], data[2]);
            buttons.push_back(Button());
            buttons.rbegin()->init(pos);
        }
    }
    in.close();

    for (Wall &wall : walls) {
        gameObjects.push_back(&wall);
    }

    for (Box &box : boxes) {
        gameObjects.push_back(&box);
    }

    for (Button &button : buttons) {
        gameObjects.push_back(&button);
    }

    for (Portal &portal : portals) {
        gameObjects.push_back(&portal);
        if (portal.hasOutline) {
            gameObjects.push_back(portal.outline);
        }
    }
}