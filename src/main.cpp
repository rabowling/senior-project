#include <PxPhysicsAPI.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

using namespace physx;

PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;
PxFoundation *gFoundation = NULL;
PxPhysics *gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;

PxRigidDynamic *gBox = NULL;
GLFWwindow *windowHandle = NULL;

void initPhysics() {
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) {
        std::cout << "PxCreateFoundation failed" << std::endl;
        exit(1);
    }

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
    if (!gPhysics) {
        std::cout << "PxCreatePhysics failed" << std::endl;
        exit(1);
    }

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    if (!gDispatcher) {
        std::cout << "PxDefaultCpuDispatcherCreate failed" << std::endl;
        exit(1);
    }

    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);
    if (!gScene) {
        std::cout << "createScene failed" << std::endl;
        exit(1);
    }
}

void initGeom() {
    PxMaterial *material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic *groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *material);
    gScene->addActor(*groundPlane);

    PxShape *shape = gPhysics->createShape(PxBoxGeometry(2, 2, 2), *material);
    gBox = gPhysics->createRigidDynamic(PxTransform(PxVec3(0, 10, 0)));
    gBox->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*gBox, 10.0f);
    gScene->addActor(*gBox);
    shape->release();
}

bool initWindow(int width, int height) {

	// Initialize glfw library
	if (!glfwInit())
	{
		return false;
	}

	//request the highest possible version of OGL - important for mac
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	windowHandle = glfwCreateWindow(width, height, "hello 3D", nullptr, nullptr);
	if (! windowHandle)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(windowHandle);

	// Initialize GLAD
	if (!gladLoadGL())
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Set vsync
	glfwSwapInterval(1);

    return true;
}

int main() {
    initPhysics();
    initGeom();
    initWindow(800, 600);

    for (int i = 0; i < 120; i++) {
        gScene->simulate(1.0f / 60.0f);
        gScene->fetchResults(true);
        PxVec3 pos = gBox->getGlobalPose().p;
        std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
        usleep(static_cast<useconds_t>(1.0 / 60.0 * 1000000));
        glfwSwapBuffers(windowHandle);
        glfwPollEvents();
    }

    glfwDestroyWindow(windowHandle);
	glfwTerminate();

    return 0;
}
