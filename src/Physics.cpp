#include "Physics.h"
#include <iostream>
#include "Application.h"
#include "Button.h"
#include "GameObject.h"
#include "Utils.h"
#include <glm/glm.hpp>

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
    pairFlags |= PxPairFlag::eMODIFY_CONTACTS;
	return PxFilterFlag::eDEFAULT;
}

void Physics::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {
    for (int i = 0; i < 2; i++) {
        PxActor *actor1 = pairHeader.actors[i];
        PxActor *actor2 = pairHeader.actors[(i+1)%2];

        GameObject *obj = static_cast<GameObject *>(actor1->userData);
        if (dynamic_cast<Button *>(obj)) {
            static_cast<Button *>(obj)->onContact(actor2);
        }
    }
}

void Physics::onContactModify(physx::PxContactModifyPair *const pairs, physx::PxU32 count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 2; j++) {
            const PxRigidActor *actor1 = pairs[i].actor[i];
            const PxRigidActor *actor2 = pairs[i].actor[(i+1)%2];
            
            if (actor1->userData != NULL) {
                GameObject *obj = static_cast<GameObject *>(pairs[i].actor[0]->userData);
                obj->onContactModify(actor2, pairs[i].contacts);
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
    sceneDesc.contactModifyCallback = this;
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