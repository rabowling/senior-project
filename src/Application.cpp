#include "Application.h"

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "Shape.h"
#include "GLSL.h"

using namespace physx;
using namespace std;
using namespace glm;

#define FPS 1.0f / 60.0f

Application app;

void Application::run(const vector<string> &args) {
    // init
    portals.reserve(MAX_PORTALS);
    buttons.reserve(32);
    boxes.reserve(32);
    walls.reserve(256);
    
    windowManager.init(1280, 720);
    physics.init();
    player.init();

    if (args.size() == 1) {
        controls.init();
    }
    else if (args.size() == 3) {
        if (args[1] == "-r") {
            controls.init(Controls::RECORD, args[2]);
        }
        else if (args[1] == "-p") {
            controls.init(Controls::PLAYBACK, args[2]);
        }
        else {
            cout << "Invalid args" << endl;
            exit(-1);
        }
    }

    loadLevel("../resources/levels/level1.txt");
    shaderManager.loadShaders("../resources/shaders");
    textureManager.loadTextures("../resources/textures");
    modelManager.loadModels("../resources/models");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    while (!glfwWindowShouldClose(windowManager.getHandle())) {
        controls.update();
        player.update(1.0f / 60.0f);
        physics.getScene()->simulate(1.0f / 60.0f);
        stepCount++;
        physics.getScene()->fetchResults(true);
        render(1.0f / 60.0f);
        glfwSwapBuffers(windowManager.getHandle());
        glfwPollEvents();
    }

    windowManager.shutdown();
}

void Application::render(float dt) {
    int width, height;
    glfwGetFramebufferSize(windowManager.getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear framebuffer.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(.12f, .34f, .56f, 1.0f);
    glStencilMask(0x00);
    // Create MVP matrices
    MatrixStack M;
    float aspect = width / (float) height;
    mat4 V = player.camera.getLookAt();
    mat4 P = glm::perspective(45.0f, aspect, 0.1f, 100.0f);

    // Render entire scene
    drawScene(P, V, player.camera);

    // Draw geometry of portals to stencil buffer
    shaderManager.bind("portal");
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, value_ptr(P));
    glUniform3f(shaderManager.getUniform("outlinecolor"), 0, 1, 1);
    glStencilMask(0xFF);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    for (int i = 0; i < portals.size(); i++) {
        glStencilFunc(GL_ALWAYS, i + 1, 0xFF);
        portals[i].draw(M);
    }

    // Render portal outline
    glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);

    for (int i = 0; i < portals.size(); i++) {
        glStencilFunc(GL_NOTEQUAL, i + 1, 0xFF);
        portals[i].drawOutline(M);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    //glEnable(GL_DEPTH_TEST);
    // Render scene through portals
    for (int i = 0; i < portals.size(); i++) {
        glStencilFunc(GL_EQUAL, i + 1, 0xFF);
        glClear(GL_DEPTH_BUFFER_BIT);
        Portal *linkedPortal = portals[i].linkedPortal;
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

void Application::drawScene(const mat4 &P, const mat4 &V, const Camera &camera) {
    MatrixStack M;

    shaderManager.bind("tex");
    textureManager.bind("marble", "Texture0");

    glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
    glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
    glUniform3f(shaderManager.getUniform("MatAmb"), 0.1, 0.18725, 0.1745);
    glUniform3f(shaderManager.getUniform("MatDif"), 0.396, 0.74151, 0.69102);
    glUniform3f(shaderManager.getUniform("MatSpec"), 0.297254, 0.30829, 0.306678);
    glUniform1f(shaderManager.getUniform("Shine"), 12.8);
    glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
    for (Box &box : boxes) {
        box.draw(M);
    }

    glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
    glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
    glUniform3f(shaderManager.getUniform("MatAmb"), 0.1, 0.18725, 0.1745);
    glUniform3f(shaderManager.getUniform("MatSpec"), 0.8, 0.8, 0);
    glUniform1f(shaderManager.getUniform("Shine"), 12.8);
    glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
    for (Button &button : buttons) {
        if (button.pressed) {
            glUniform3f(shaderManager.getUniform("MatDif"), 1, 0, 0);
        } else {
            glUniform3f(shaderManager.getUniform("MatDif"), 1, 0.8, 0);
        }
        button.draw(M);
    }

    // Set up wall shader colors here
    shaderManager.bind("wall");
    textureManager.bind("concrete", "Texture0");
    glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
	glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
    glUniform3f(shaderManager.getUniform("MatAmb"), 0.19225, 0.19225, 0.19225);
    glUniform3f(shaderManager.getUniform("MatDif"), 0.50754, 0.50754, 0.50754);
    glUniform3f(shaderManager.getUniform("MatSpec"), 0.508273, 0.508273, 0.508273);
    glUniform1f(shaderManager.getUniform("Shine"), 51.2);
    glUniform3fv(shaderManager.getUniform("viewPos"), 1, glm::value_ptr(camera.eye));
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V));
    for (int i = 0; i < walls.size(); i++) {
        walls[i].draw(M);
    }
}

void Application::loadLevel(string levelFile) {
    ifstream in;
    in.open("../resources/levels/level1.txt");
    string line;
    vector<int> portalIds;
    int initialPortals = portals.size();
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
            portals.push_back(Portal(pos, scale, rot, "world_portal"));
            Portal *portal = &portals[portals.size() - 1];
            portal->setPosition(pos, rot);

            // link portals
            int linkedPortalId = intData[1];
            for (int i = 0; i < portalIds.size(); i++) {
                if (portalIds[i] == linkedPortalId) {
                    portal->linkPortal(&portals[i+initialPortals]);
                    break;
                }
            }

            portalIds.push_back(intData[0]);
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
}