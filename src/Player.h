#pragma once
#include "Physics.h"
#include "WindowManager.h"
#include "Camera.h"
#include "Portal.h"
#include <PxPhysicsAPI.h>

class Player : public physx::PxUserControllerHitReport, physx::PxQueryFilterCallback
{
    public:
        void update(float dt);
        void init();
        void setPosition(float x, float y, float z);
        
        virtual void onShapeHit(const physx::PxControllerShapeHit &hit);
        virtual void onControllerHit(const physx::PxControllersHit &hit) {}
        virtual void onObstacleHit(const physx::PxControllerObstacleHit &hit) {}
        virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData, const physx::PxShape *shape,
            const physx::PxRigidActor *actor, physx::PxHitFlags &queryFlags);
        virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData, const physx::PxQueryHit &hit) {}
        Camera camera;
        Portal *portals[2] = { nullptr, nullptr };
    private:
        physx::PxController *mController;
        physx::PxScene *mScene;
        WindowManager *mWindowManager;
        physx::PxVec3 origin;
        physx::PxVec3 unitDir;
        physx::PxVec3 lastLocation;
        physx::PxVec3 velocity;
        float mWalkSpeed = 5.0f;
        physx::PxRigidBody *heldItem = NULL;
        float jumpSpeed = 10;
        float radius = 1;
        float height = 2;
        
        enum RaycastMode { PICK_UP, FIRE_PORTAL };
        RaycastMode raycastMode;
};