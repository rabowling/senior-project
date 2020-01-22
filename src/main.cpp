#include <PxPhysicsAPI.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "Program.h"
#include "MatrixStack.h"
#include "GLSL.h"
#include "Shape.h"
#include "WindowManager.h"
#include "Physics.h"
#include "Camera.h"
#include "ShaderManager.h"

using namespace physx;

Physics physics;
Camera camera;
ShaderManager shaderManager;

PxRigidDynamic *gBox = NULL;
PxRigidStatic *gGroundPlane = NULL;

WindowManager *windowManager;
std::shared_ptr<Shape> boxShape;

void render() {
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.12f, .34f, .56f, 1.0f);


    /* Leave this code to just draw the meshes alone */
    float aspect = width/(float)height;

    // Create the matrix stacks
    auto P = std::make_shared<MatrixStack>();
    auto M = std::make_shared<MatrixStack>();
    auto V = std::make_shared<MatrixStack>();
    // Apply perspective projection.
    P->pushMatrix();
    P->perspective(45.0f, aspect, 0.01f, 100.0f);
    camera.lookAt(V);

    M->pushMatrix();
        M->loadIdentity();

        // Draw spiders
        shaderManager.bind("mat");
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

        shaderManager.unbind();

    M->popMatrix();
}

void initGeom(std::string resourceDirectory) {
    // Box geometry
    boxShape = std::make_shared<Shape>();
    boxShape->loadMesh(resourceDirectory + "/cube.obj");
    boxShape->init();

    // Physics ground plane
    PxMaterial *material = physics.getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);
    gGroundPlane = PxCreatePlane(*(physics.getPhysics()), PxPlane(0, 1, 0, 0), *material);
    physics.getScene()->addActor(*gGroundPlane);

    // Physics box
    PxShape *shape = physics.getPhysics()->createShape(PxBoxGeometry(2, 2, 2), *material);
    gBox = physics.getPhysics()->createRigidDynamic(PxTransform(PxVec3(10, 10, 0)));
    gBox->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*gBox, 10.0f);
    physics.getScene()->addActor(*gBox);
    shape->release();
}

int main() {

    windowManager = new WindowManager();
    windowManager->init(512, 512);
    physics.init();
    camera.init(windowManager, glm::vec3(15, 15, 15), glm::vec3(0, 0, -1));

    initGeom("../resources");
    shaderManager.loadShaders("../shaders");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(windowManager->getHandle())) {
        camera.update(1.0f / 60.0f);
        render();
        physics.getScene()->simulate(1.0f / 60.0f);
        physics.getScene()->fetchResults(true);
        glfwSwapBuffers(windowManager->getHandle());
        glfwPollEvents();
    }

    windowManager->shutdown();

    delete windowManager;

    return 0;
}
