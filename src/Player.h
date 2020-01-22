#pragma once
#include "Physics.h"
#include "WindowManager.h"
#include "Camera.h"

class Player {
    public:
        Player(WindowManager *windowManager, Physics *physics, Camera *camera);
        void update(float dt);
        
    private:
        physx::PxController *mController;
        physx::PxScene *mScene;
        Camera *mCamera;
        WindowManager *mWindowManager;
        physx::PxVec3 velocity;
        physx::PxVec3 origin;
        physx::PxVec3 unitDir;
        physx::PxVec3 lastLocation;
        float gravity = -9.8f;
        float mWalkSpeed = 5.0f;
        bool isJumping = false;
        bool isCasting = false;
        physx::PxRigidBody *currentTarget = NULL;
};