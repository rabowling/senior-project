#pragma once
#include "Physics.h"
#include "WindowManager.h"
#include "Camera.h"

class Player {
    public:
        void update(float dt);
        void init();
        
    private:
        physx::PxController *mController;
        physx::PxScene *mScene;
        Camera *mCamera;
        WindowManager *mWindowManager;
        physx::PxVec3 origin;
        physx::PxVec3 unitDir;
        physx::PxVec3 lastLocation;
        physx::PxVec3 velocity;
        float gravity = -9.8f;
        float mWalkSpeed = 5.0f;
        physx::PxRigidBody *heldItem = NULL;
        float jumpSpeed = 10;
};