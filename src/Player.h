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
        Camera *mCamera;
        WindowManager *mWindowManager;
        physx::PxVec3 velocity;
        float gravity = -9.8f;
        float mWalkSpeed = 5.0f;
};