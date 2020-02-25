#include "Physics.h"
#include <iostream>
#include "Application.h"
#include "Button.h"

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
    for (int i = 0; i < 2; i++) {
        PxActor *actor1 = pairHeader.actors[i];
        PxActor *actor2 = pairHeader.actors[(i+1)%2];

        if (actor1->userData != nullptr) {
            for (Button &button : app.buttons) {
                if (button.body == actor1) {
                    static_cast<Button *>(actor1->userData)->onContact(actor2);
                }
            }
        }
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