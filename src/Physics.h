#pragma once

#include <PxPhysicsAPI.h>

class Physics
{
public:
    void init();
    physx::PxScene *getScene();
    physx::PxPhysics *getPhysics();
private:
    physx::PxDefaultAllocator mAllocator;
    physx::PxDefaultErrorCallback mErrorCallback;
    physx::PxFoundation *mFoundation;
    physx::PxPhysics *mPhysics;
    physx::PxDefaultCpuDispatcher *mDispatcher;
    physx::PxScene *mScene;
};