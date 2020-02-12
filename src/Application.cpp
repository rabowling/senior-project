#include "Application.h"

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include "Shape.h"

using namespace physx;
using namespace std;

Application app;

void Application::run(const vector<string> &args) {
    // init
    windowManager.init(1280, 720);
    physics.init();
    player.init();
    camera.init(glm::vec3(15, 15, 15), glm::vec3(0, 0, -1));

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

    initGeom("../resources");
    shaderManager.loadShaders("../shaders");
    textureManager.loadTextures("../resources/textures");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    while (!glfwWindowShouldClose(windowManager.getHandle())) {
        controls.update();
        camera.update(1.0f / 60.0f);
        render(1.0f / 60.0f);
        player.update(1.0f / 60.0f);
        physics.getScene()->simulate(1.0f / 60.0f);
        stepCount++;
        physics.getScene()->fetchResults(true);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.12f, .34f, .56f, 1.0f);

    /* Leave this code to just draw the meshes alone */
    float aspect = width/(float)height;

    // Create the matrix stacks
    auto P = std::make_shared<MatrixStack>();
    auto V = std::make_shared<MatrixStack>();
    // Apply perspective projection.
    P->pushMatrix();
    P->perspective(45.0f, aspect, 0.01f, 100.0f);
    camera.lookAt(V);

    // Render entire scene
    drawScene(P, V);

    /*
    // All fragments update stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // draw portal 1

    // Disable updating stencil buffer
    glStencilMask(0x00);

    // Move the camera for portal 1

    // Only draw area for portal
    glStencilFunc(GL_EQUAL, 1, 0xFF);

    // drawScene(P, V);

    // All fragments update stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // draw portal 2

    // Disable updating stencil buffer
    glStencilMask(0x00);

    // Move the camera for portal 2

    // Only draw area for portal
    glStencilFunc(GL_EQUAL, 1, 0xFF);

    // drawScene(P, V);
    */
}

void Application::drawScene(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> V) {
    auto M = std::make_shared<MatrixStack>();

    shaderManager.bind("tex");
    textureManager.bind("marble", "Texture0");

    M->pushMatrix();
        M->loadIdentity();

        // Draw spiders
        glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
		glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform3f(shaderManager.getUniform("MatAmb"), 0.1, 0.18725, 0.1745);
        glUniform3f(shaderManager.getUniform("MatDif"), 0.396, 0.74151, 0.69102);
        glUniform3f(shaderManager.getUniform("MatSpec"), 0.297254, 0.30829, 0.306678);
        glUniform1f(shaderManager.getUniform("Shine"), 12.8);
        glUniform3f(shaderManager.getUniform("viewPos"), camera.eye.x, camera.eye.y, camera.eye.z);
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V->topMatrix()));

        PxTransform t = gBox->getGlobalPose();
        M->translate(glm::vec3(t.p.x, t.p.y, t.p.z));
        M->rotate(glm::quat(t.q.w, t.q.x, t.q.y, t.q.z));
        M->scale(2);
        glUniformMatrix4fv(shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
        boxShape->draw(shaderManager.getActive());
    M->popMatrix();
    M->pushMatrix();
        t = gBox2->getGlobalPose();
        M->translate(glm::vec3(t.p.x, t.p.y, t.p.z));
        M->rotate(glm::quat(t.q.w, t.q.x, t.q.y, t.q.z));
        glUniformMatrix4fv(shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
        boxShape->draw(shaderManager.getActive());
    M->popMatrix();
    M->pushMatrix();
        glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
		glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
        glUniform3f(shaderManager.getUniform("MatAmb"), 0.1, 0.18725, 0.1745);
        if (buttonPressed) {
            glUniform3f(shaderManager.getUniform("MatDif"), 1, 0, 0);
        } else {
            glUniform3f(shaderManager.getUniform("MatDif"), 1, 0.8, 0);
        }
        glUniform3f(shaderManager.getUniform("MatSpec"), 0.8, 0.8, 0);
        glUniform1f(shaderManager.getUniform("Shine"), 12.8);
        glUniform3f(shaderManager.getUniform("viewPos"), camera.eye.x, camera.eye.y, camera.eye.z);
        glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V->topMatrix()));
        t = gButton->getGlobalPose();
        M->translate(glm::vec3(t.p.x, t.p.y, t.p.z));
        M->rotate(glm::quat(t.q.w, t.q.x, t.q.y, t.q.z));
        glUniformMatrix4fv(shaderManager.getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
        cylinderShape->draw(shaderManager.getActive());
    M->popMatrix();

    // Set up wall shader colors here
    shaderManager.bind("wall");
    textureManager.bind("concrete", "Texture0");
    glUniform3f(shaderManager.getUniform("dirLightDir"), 0, 1, 1);
	glUniform3f(shaderManager.getUniform("dirLightColor"), 1, 1, 1);
    glUniform3f(shaderManager.getUniform("MatAmb"), 0.19225, 0.19225, 0.19225);
    glUniform3f(shaderManager.getUniform("MatDif"), 0.50754, 0.50754, 0.50754);
    glUniform3f(shaderManager.getUniform("MatSpec"), 0.508273, 0.508273, 0.508273);
    glUniform1f(shaderManager.getUniform("Shine"), 51.2);
    glUniform3f(shaderManager.getUniform("viewPos"), camera.eye.x, camera.eye.y, camera.eye.z);
    glUniformMatrix4fv(shaderManager.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glUniformMatrix4fv(shaderManager.getUniform("V"), 1, GL_FALSE, glm::value_ptr(V->topMatrix()));
    for (int i = 0; i < walls.size(); i++) {
        walls[i].draw(shaderManager, M);
    }
}

void Application::initGeom(std::string resourceDirectory) {
    // Box geometry
    boxShape = std::make_shared<Shape>();
    boxShape->loadMesh(resourceDirectory + "/cube.obj");
    boxShape->init();

    // Plane geometry
    planeShape = std::make_shared<Shape>();
    planeShape->loadMesh(resourceDirectory + "/plane.obj");
    planeShape->init();

    // Cylinder geometry
    cylinderShape = std::make_shared<Shape>();
    cylinderShape->loadMesh(resourceDirectory + "/cylinder.obj");
    cylinderShape->init();

    // Physics ground plane
    PxMaterial *material = physics.getPhysics()->createMaterial(0.3f, 0.3f, 0.3f);
    //gGroundPlane = PxCreatePlane(*(physics.getPhysics()), PxPlane(0, 1, 0, 0), *material);
    //physics.getScene()->addActor(*gGroundPlane);

    // Physics box
    PxShape *shape = physics.getPhysics()->createShape(PxBoxGeometry(2, 2, 2), *material);
    gBox = physics.getPhysics()->createRigidDynamic(PxTransform(PxVec3(10, 10, 25)));
    gBox->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*gBox, 10.0f);
    physics.getScene()->addActor(*gBox);
    shape->release();

    PxShape *shape2 = physics.getPhysics()->createShape(PxBoxGeometry(1, 1, 1), *material);
    gBox2 = physics.getPhysics()->createRigidDynamic(PxTransform(PxVec3(15, 10, 35)));
    gBox2->attachShape(*shape2);
    PxRigidBodyExt::updateMassAndInertia(*gBox2, 10.0f);
    physics.getScene()->addActor(*gBox2);
    shape2->release();

    // Button
    PxShape *shape3 = physics.getPhysics()->createShape(PxBoxGeometry(1, 0.4, 1), *material);
    gButton = physics.getPhysics()->createRigidStatic(PxTransform(PxVec3(13, 0.4, 40)));
    gButton->attachShape(*shape3);
    physics.getScene()->addActor(*gButton);
    shape3->release();

    // Four outer walls
    makeWall(PxVec3(35.0f, 6.0f, 0.0f), PxVec3(50.0f, 6.0f, 1.0f), PxQuat(M_PI/2, PxVec3(0,1,0)));
    makeWall(PxVec3(-35.0f, 6.0f, 0.0f), PxVec3(50.0f, 6.0f, 1.0f), PxQuat(M_PI/2, PxVec3(0,1,0)));
    makeWall(PxVec3(0.0f, 6.0f, -50.0f), PxVec3(35.0f, 6.0f, 1.0f), PxQuat(0, PxVec3(0,1,0)));
    makeWall(PxVec3(0.0f, 6.0f, 50.0f), PxVec3(35.0f, 6.0f, 1.0f), PxQuat(0, PxVec3(0,1,0)));

    // Interior middle wall
    makeWall(PxVec3(5.0f, 6.0f, -10.0f), PxVec3(40.0f, 6.0f, 0.5f), PxQuat(M_PI/2, PxVec3(0,1,0)));

    // Maze walls
    makeWall(PxVec3(11.0f, 6.0f, 15.0f), PxVec3(6.0f, 6.0f, 0.5f), PxQuat(0, PxVec3(0,1,0)));
    makeWall(PxVec3(29.0f, 6.0f, 15.0f), PxVec3(6.0f, 6.0f, 0.5f), PxQuat(0, PxVec3(0,1,0)));
    makeWall(PxVec3(17.0f, 6.0f, 5.0f), PxVec3(12.0f, 6.0f, 0.5f), PxQuat(0, PxVec3(0,1,0)));
    makeWall(PxVec3(22.0f, 6.0f, -5.0f), PxVec3(12.0f, 6.0f, 0.5f), PxQuat(0, PxVec3(0,1,0)));

    // Lower floor
    makeWall(PxVec3(20.0f, -1.0f, 20.0f), PxVec3(15.0f, 1.0f, 30.0f), PxQuat(0, PxVec3(0,1,0)));

    // Upper floors
    makeWall(PxVec3(-15.0f, 3.5f, 40.0f), PxVec3(20.0f, 3.5f, 10.0f), PxQuat(0, PxVec3(0,1,0)));
    makeWall(PxVec3(-15.0f, 3.5f, -15.0f), PxVec3(20.0f, 3.5f, 35.0f), PxQuat(0, PxVec3(0,1,0)));
}

void Application::makeWall(PxVec3 pos, PxVec3 size, PxQuat rot) {
    Wall w;
    w.init(pos, size, rot, physics, boxShape);
    walls.push_back(w);
}