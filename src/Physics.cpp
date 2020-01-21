#include "Physics.h"
#include <iostream>

using namespace physx;

void Physics::init() {
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
    if (!mFoundation) {
        std::cout << "PxCreateFoundation failed" << std::endl;
        exit(1);
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale());
    if (!mPhysics) {
        std::cout << "PxCreatePhysics failed" << std::endl;
        exit(1);
    }

    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    if (!mDispatcher) {
        std::cout << "PxDefaultCpuDispatcherCreate failed" << std::endl;
        exit(1);
    }

    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene) {
        std::cout << "createScene failed" << std::endl;
        exit(1);
    }
}

PxScene *Physics::getScene() {
    return mScene;
}

PxPhysics *Physics::getPhysics() {
    return mPhysics;
}