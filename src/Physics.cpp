#include "Physics.h"
#include <iostream>
#include "Application.h"

using namespace physx;

PxFilterFlags myFilterShader(
        PxFilterObjectAttributes attributes0,
        PxFilterData filterData0, 
        PxFilterObjectAttributes attributes1,
        PxFilterData filterData1,
        PxPairFlags& pairFlags,
        const void* constantBlock,
        PxU32 constantBlockSize)
{
    pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
    pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
	return PxFilterFlag::eDEFAULT;
}

void Physics::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {
    if (pairHeader.actors[0] == app.gButton || pairHeader.actors[1] == app.gButton) {
        app.buttonPressed = true;
    }
}

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
    sceneDesc.gravity = PxVec3(0.0f, GRAVITY, 0.0f);
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    if (!mDispatcher) {
        std::cout << "PxDefaultCpuDispatcherCreate failed" << std::endl;
        exit(1);
    }

    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = myFilterShader;
    sceneDesc.simulationEventCallback = this;
    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene) {
        std::cout << "createScene failed" << std::endl;
        exit(1);
    }

    mManager = PxCreateControllerManager(*mScene);

    defaultMaterial = mPhysics->createMaterial(0.3f, 0.3f, 0.3f);
}

PxScene *Physics::getScene() {
    return mScene;
}

PxPhysics *Physics::getPhysics() {
    return mPhysics;
}

PxControllerManager *Physics::getControllerManager() {
    return mManager;
}