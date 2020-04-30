#pragma once
#include "Physics.h"
#include "WindowManager.h"
#include "Camera.h"
#include "Portal.h"
#include "MatrixStack.h"
#include <PxPhysicsAPI.h>
#include <vector>

class Player : public physx::PxUserControllerHitReport
{
    public:
        void update(float dt);
        void init();
        void setPosition(float x, float y, float z);
        void draw(MatrixStack &M);
        
        virtual void onShapeHit(const physx::PxControllerShapeHit &hit);
        virtual void onControllerHit(const physx::PxControllersHit &hit) {}
        virtual void onObstacleHit(const physx::PxControllerObstacleHit &hit) {}

        Camera camera;
        Portal *portals[2] = { nullptr, nullptr };
        
    private:
        physx::PxController *mController;
        physx::PxVec3 origin;
        physx::PxVec3 unitDir;
        physx::PxVec3 lastLocation;
        physx::PxVec3 velocity;
        float mWalkSpeed = 5.0f;
        physx::PxRigidBody *heldItem = NULL;
        float jumpSpeed = 10;
        float radius = 1;
        float height = 2;
        glm::vec3 camOffset = glm::vec3(0, 1, 0);
        
        enum RaycastMode { USE, FIRE_PORTAL };
        RaycastMode raycastMode;

        std::vector<Portal *> touchingPortals;
        std::vector<Portal *> prevTouchingPortals;

        class RaycastFilterCallback : public physx::PxQueryFilterCallback {
            public:
                RaycastFilterCallback(Player *parent);

            private:
                Player *parent;
                virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData, const physx::PxShape *shape,
                    const physx::PxRigidActor *actor, physx::PxHitFlags &queryFlags);
                virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData, const physx::PxQueryHit &hit) {}
        };
        std::unique_ptr<RaycastFilterCallback> raycastFilterCallback;

        class MoveFilterCallback : public physx::PxQueryFilterCallback {
            public:
                MoveFilterCallback(Player *parent);

            private:
                Player *parent;
                virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData, const physx::PxShape *shape,
                    const physx::PxRigidActor *actor, physx::PxHitFlags &queryFlags);
                virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData, const physx::PxQueryHit &hit) {}
        };
        std::unique_ptr<MoveFilterCallback> moveFilterCallback;
};