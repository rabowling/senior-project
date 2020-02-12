#pragma once
#include "Physics.h"
#include "WindowManager.h"
#include "Camera.h"
#include <PxPhysicsAPI.h>

class Player : public physx::PxUserControllerHitReport {
    public:
        void update(float dt);
        void init();
        void setPosition(float x, float y, float z);
        
        virtual void onShapeHit(const physx::PxControllerShapeHit &hit);
        virtual void onControllerHit(const physx::PxControllersHit &hit);
        virtual void onObstacleHit(const physx::PxControllerObstacleHit &hit);
    private:
        physx::PxController *mController;
        physx::PxScene *mScene;
        Camera *mCamera;
        WindowManager *mWindowManager;
        physx::PxVec3 origin;
        physx::PxVec3 unitDir;
        physx::PxVec3 lastLocation;
        physx::PxVec3 velocity;
        float mWalkSpeed = 5.0f;
        physx::PxRigidBody *heldItem = NULL;
        float jumpSpeed = 10;
};