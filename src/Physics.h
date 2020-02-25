#pragma once

#include <PxPhysicsAPI.h>

#define GRAVITY -9.81

class Physics : public physx::PxSimulationEventCallback
{
public:
    void init();
    physx::PxScene *getScene();
    physx::PxPhysics *getPhysics();
    physx::PxControllerManager *getControllerManager();

    virtual	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
    virtual	void onTrigger(physx::PxTriggerPair*, physx::PxU32) {}
    virtual void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) {}
    virtual void onWake(physx::PxActor** , physx::PxU32 ) {}
    virtual void onSleep(physx::PxActor** , physx::PxU32 ){}
    virtual void onAdvance(const physx::PxRigidBody*const*, const physx::PxTransform*, const physx::PxU32) {}

    physx::PxMaterial *defaultMaterial;
private:
    physx::PxDefaultAllocator mAllocator;
    physx::PxDefaultErrorCallback mErrorCallback;
    physx::PxFoundation *mFoundation;
    physx::PxPhysics *mPhysics;
    physx::PxDefaultCpuDispatcher *mDispatcher;
    physx::PxScene *mScene;
    physx::PxControllerManager *mManager;
};