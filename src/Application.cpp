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
#include "Utils.h"

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

    for (int i = 0; i < NUM_PORTALS; i++) {
        portalLights[i].direction = vec3(0,0,0);
        portalLights[i].position = vec3(0,0,0);
        portalLights[i].portal = NULL;
    }

    loadLevel("../resources/levels/level1.txt");
    shaderManager.loadShaders("../resources/shaders");
    textureManager.loadTextures("../resources/textures");
    modelManager.loadModels("../resources/models");
    materialManager.loadMaterials();

    initCubemap();
    initDepthmaps();

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
        updatePortalLights();
        render(1.0f / 60.0f);
        glfwSwapBuffers(windowManager.getHandle());
        glfwPollEvents();

        if (renderMode == RENDER_RAYTRACE) {
            string numString = to_string(stepCount);
            numString = string(5 - numString.length(), '0') + numString;
            renderRT(1920, 1080, "render/frame" + numString + ".png");
            if (controls.playbackFinished()) {
                break;
            }
        }
        stepCount++;
    }

    windowManager.shutdown();
}

void Application::updatePortalLights() {
    for (int i = 0; i < NUM_PORTALS; i++) {
        PxRaycastBuffer hit;
        PxVec3 origin = glm2px(portalLights[i].position + portalLights[i].direction);
        PxVec3 direction = glm2px(normalize(lightPos - (px2glm(origin))));
        PxReal maxDist = distance(px2glm(origin), lightPos);
        bool success = physics.getScene()->raycast(origin, direction, maxDist, hit);
        float newIntensity = 1.0f;
        if (success) {
            newIntensity = 0.0f;
        } else {
            newIntensity = 1.0f;
        }
        if (portalLights[i].portal != NULL) {
            portalLights[i].portal->linkedPortal->intensity = newIntensity;
        }
    }
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

    renderToCubemap(P, V, player.camera);
    renderToDepthmap(P, V, player.camera, "depth");

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear framebuffer.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);
    glStencilMask(0x00);

    if (controls.isHeld(Controls::DEBUG_LIGHT)) {
        renderToDepthmap(P, V, player.camera, "depthdebug");
        return;
    }

    // Create MVP matrices
    MatrixStack M;

    // Render entire scene
    renderingFP = true;
    drawScene(P, V, player.camera);
    renderingFP = false;

    // Draw geometry of portals to stencil buffer
    shaderManager.bind("portal");
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, value_ptr(P));
    glStencilMask(0xFF);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    int i = 1;
    for (Portal &portal : portals) {
        if (!portal.facing(player.camera.eye)) {
            continue;
        }

        glStencilFunc(GL_ALWAYS, i++, 0xFF);
        portal.draw(M);
    }

    // Render portal outline
    glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);

    i = 1;
    for (Portal &portal : portals) {
        if (!portal.facing(player.camera.eye)) {
            continue;
        }

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
        if (!portal.facing(player.camera.eye)) {
            continue;
        }

        glStencilFunc(GL_EQUAL, i++, 0xFF);
        glClear(GL_DEPTH_BUFFER_BIT);
        Portal *linkedPortal = portal.linkedPortal;
        linkedPortal->updateCamera(player.camera);
        mat4 portalV = linkedPortal->camera.getLookAt();
        mat4 portalP = linkedPortal->modifyProjectionMatrix(P, portalV);
        drawScene(portalP, portalV, linkedPortal->camera);

        shaderManager.bind("portal");
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(portalP));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(portalV));
        glEnable(GL_POLYGON_OFFSET_FILL);
        for (Portal &portal : portals) {
            portal.draw(M);
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void Application::renderToDepthmap(const mat4 &P, const mat4 &V, const Camera &camera, string shader) {
    app.renderingCubemap = true;

    bool debug = (shader == "depthdebug");

    shaderManager.bind(shader);
    int iters = debug ? 1 : NUM_PORTALS;
    for (int i = 0; i < iters; i++) {
        if (!debug) {
            CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i+1]));
            CHECKED_GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
            CHECKED_GL_CALL(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
        }

        mat4 LP = ortho(-10.0, 10.0, -10.0, 10.0, 0.1, 50.0);
        mat4 LV;
        
        if (!debug) {
            LV = lookAt(portalLights[i].position, 
                         portalLights[i].position + normalize(portalLights[i].direction),
                         vec3(0, 1, 0));
        } else {
            LV = lookAt(portalLights[0].position, 
                         portalLights[0].position + normalize(portalLights[0].direction),
                         vec3(0, 1, 0));
        }
        
        CHECKED_GL_CALL(glUniformMatrix4fv(shaderManager.getUniform("LP"), 1, GL_FALSE, value_ptr(LP)));
		CHECKED_GL_CALL(glUniformMatrix4fv(shaderManager.getUniform("LV"), 1, GL_FALSE, value_ptr(LV)));
        
        if (!debug) {
            CHECKED_GL_CALL(glCullFace(GL_FRONT));
        }
        drawScene(P, V, camera);

        if (!debug) {
            CHECKED_GL_CALL(glCullFace(GL_BACK));
            CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }
    }

    app.renderingCubemap = false;
}

void Application::renderToCubemap(const mat4 &P, const mat4 &V, const Camera &camera) {
    app.renderingCubemap = true;
    
    CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[0]));
    CHECKED_GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));

    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    mat4 shadowProj = perspective(radians(90.0f), aspect, near, far);

    vec3 curLightPos = lightPos;

    vector<mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * lookAt(curLightPos, curLightPos + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));

    CHECKED_GL_CALL(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[0]));
    CHECKED_GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
    shaderManager.bind("cubemap");

    for (unsigned int i = 0; i < 6; i++) {
        CHECKED_GL_CALL(glUniformMatrix4fv(shaderManager.getUniform("shadowMatrices[" + to_string(i) + "]"), 1, GL_FALSE, value_ptr(shadowTransforms[i])));
    }
    CHECKED_GL_CALL(glUniform1f(shaderManager.getUniform("farPlane"), far));
    CHECKED_GL_CALL(glUniform3fv(shaderManager.getUniform("lightPos"), 1, value_ptr(curLightPos)));

    CHECKED_GL_CALL(glCullFace(GL_FRONT));
    drawScene(P, V, camera);
    CHECKED_GL_CALL(glCullFace(GL_BACK));
    CHECKED_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    app.renderingCubemap = false;
}

void Application::drawScene(const mat4 &P, const mat4 &V, const Camera &camera) {
    MatrixStack M;

    if (!renderingCubemap) {
        shaderManager.bind("tex");
        glUniform3fv(shaderManager.getUniform("pointLightPos"), 1, value_ptr(lightPos));
        for (int i = 0; i < NUM_PORTALS; i++) {
            glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].pos"), 1, value_ptr(portalLights[i].position));
            glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].dir"), 1, value_ptr(portalLights[i].direction));
            glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].innerCutoff"), INNER_CUTOFF);
            glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].outerCutoff"), OUTER_CUTOFF);
            if (portalLights[i].portal != NULL) {
                glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].intensity"), portalLights[i].portal->intensity);
            } else {
                glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].intensity"), 0.0f);
            }
        }
        glUniform1f(shaderManager.getUniform("farPlane"), far);
        glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
        for (int i = 0; i < NUM_PORTALS; i++) {
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + 2 + i));
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, depthMaps[i]));
        }
        CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE1));
        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap));
    }
    for (Box &box : boxes) {
        box.draw(M);
    }

    // if (!renderingCubemap) {
    //     glUniform3fv(shaderManager.getUniform("pointLightPos"), 1, value_ptr(lightPos));
    //     for (int i = 0; i < NUM_PORTALS; i++) {
    //         glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].pos"), 1, value_ptr(portalLights[i].position));
    //         glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].dir"), 1, value_ptr(portalLights[i].direction));
    //         glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].innerCutoff"), INNER_CUTOFF);
    //         glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].outerCutoff"), OUTER_CUTOFF);
    //         glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].intensity"), 1.0f);
    //     }
    //     glUniform1f(shaderManager.getUniform("farPlane"), far);
    //     glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
    //     glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
    //     glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
    //     glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    //     for (int i = 1; i < NUM_PORTALS + 1; i++) {
    //         CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE1 + i));
    //         CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, depthMaps[i-1]));
    //     }
    // }
    for (Button &button : buttons) {
        button.draw(M);
    }
    if (!renderingFP) {
        player.draw(M);
    }

    if (!renderingCubemap) {
        // Set up wall shader colors here
        shaderManager.bind("wall");
        glUniform3fv(shaderManager.getUniform("pointLightPos"), 1, value_ptr(lightPos));
        for (int i = 0; i < NUM_PORTALS; i++) {
            glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].pos"), 1, value_ptr(portalLights[i].position));
            glUniform3fv(shaderManager.getUniform("portalLights[" + to_string(i) + "].dir"), 1, value_ptr(portalLights[i].direction));
            glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].innerCutoff"), INNER_CUTOFF);
            glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].outerCutoff"), OUTER_CUTOFF);
            if (portalLights[i].portal != NULL) {
                glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].intensity"), portalLights[i].portal->intensity);
            } else {
                glUniform1f(shaderManager.getUniform("portalLights[" + to_string(i) + "].intensity"), 0.0f);
            }
        }
	    glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform1f(shaderManager.getUniform("farPlane"), far);
        glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
        for (int i = 0; i < NUM_PORTALS; i++) {
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + 2 + i));
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, depthMaps[i]));
        }
        CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE1));
        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap));
    }
    for (Wall &wall : walls) {
        wall.draw(M);
    }
}

void Application::initCubemap() {
    glGenFramebuffers(1, &depthMapFBO[0]);
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

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[0]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::initDepthmaps() {

    int pid = shaderManager.getPid("tex");
    int tex1 = glGetUniformLocation(pid, "depthMapPortal1");
    int tex2 = glGetUniformLocation(pid, "depthMapPortal2");
    int tex3 = glGetUniformLocation(pid, "depthMapPortal3");
    int tex4 = glGetUniformLocation(pid, "depthMapPortal4");
    shaderManager.bind("tex");
    glUniform1i(tex1, 2);
    glUniform1i(tex2, 3);
    glUniform1i(tex3, 4);
    glUniform1i(tex4, 5);
    shaderManager.unbind();

    pid = shaderManager.getPid("wall");
    tex1 = glGetUniformLocation(pid, "depthMapPortal1");
    tex2 = glGetUniformLocation(pid, "depthMapPortal2");
    tex3 = glGetUniformLocation(pid, "depthMapPortal3");
    tex4 = glGetUniformLocation(pid, "depthMapPortal4");
    shaderManager.bind("wall");
    glUniform1i(tex1, 2);
    glUniform1i(tex2, 3);
    glUniform1i(tex3, 4);
    glUniform1i(tex4, 5);
    shaderManager.unbind();

    for (int i = 0; i < NUM_PORTALS; i++) {
        glGenFramebuffers(1, &depthMapFBO[i+1]);
		glGenTextures(1, &depthMaps[i]);

		glBindTexture(GL_TEXTURE_2D, depthMaps[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
             		 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i+1]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMaps[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Application::loadLevel(string levelFile) {
    ifstream in;
    in.open("../resources/levels/level1.txt");
    string line;
    map<int, Portal *> portalIdMap;
    bool loadedPortal1 = false;
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

            if (!loadedPortal1) {
                portalLights[WORLD_PORTAL_1].direction = portal->getForward();
                portalLights[WORLD_PORTAL_1].position = pos;
                portalLights[WORLD_PORTAL_1].portal = portal;
                loadedPortal1 = true;
            } else {
                portalLights[WORLD_PORTAL_2].direction = portal->getForward();
                portalLights[WORLD_PORTAL_2].position = pos;
                portalLights[WORLD_PORTAL_2].portal = portal;
            }

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
        else if (type == "light") {
            float data[6];
            for (int i = 0; i < 6; i++) {
                iss >> data[i];
                iss.ignore();
            }
            Light light;
            light.position = vec3(data[0], data[1], data[2]);
            light.intensity = vec3(data[3], data[4], data[5]);
            lights.push_back(light);
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