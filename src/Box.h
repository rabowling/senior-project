#pragma once

#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>
#include "MatrixStack.h"
#include "GameObject.h"
#include "Material.h"
#include "Portal.h"

class Box : public GameObject
{
public:
    void init(physx::PxVec3 location, physx::PxVec3 scale, physx::PxQuat rotation);
    void draw(MatrixStack &M);
    void update(float dt);
    virtual void onContactModify(const physx::PxRigidActor *actor, physx::PxContactSet &contacts);
    float density = 10.0f;
    physx::PxRigidDynamic *body;
    
    virtual Shape *getModel() const;
    virtual glm::mat4 getTransform() const;
    virtual Material *getMaterial() const;
    
private:
    std::vector<Portal *> touchingPortals;
    std::vector<Portal *> prevTouchingPortals;
    glm::vec3 scale;
    physx::PxVec3 startPos;
    physx::PxQuat startRot;
};