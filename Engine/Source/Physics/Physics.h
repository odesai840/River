#ifndef PHYSICS_H
#define PHYSICS_H

#include "Math/Math.h"
#include "Renderer/Entity.h"
#include <vector>

namespace RiverCore{

class Physics {
public:
    Physics() = default;
    ~Physics() = default;

    // Function to set gravity
    void SetGravity(const float gravity) { gravityAmount = gravity; }
    // Function to get gravity
    float GetGravity() const { return gravityAmount; }
    // Function to get gravity vector
    Vec2 GetGravityVector() const { return Vec2(0.0f, gravityAmount); }

    // Function to update physics
    void UpdatePhysics(std::vector<Entity>& entities, float fixedDeltaTime);

    // Function to update collisions
    void UpdateCollisions(std::vector<Entity>& entities);
    // Uses Axis-Aligned Bounding Box (AABB) collision detection to check for collisions between two entities
    bool CheckAABBCollision(const Entity& a, const Entity& b) const;

    // Applies a force to an entity
    void ApplyForce(Entity& entity, const Vec2& force);
    // Applies an impulse to an entity
    void ApplyImpulse(Entity& entity, const Vec2& impulse);
    // Sets the velocity of an entity
    void SetVelocity(Entity& entity, const Vec2& velocity);

private:
    // Gravity constant
    float gravityAmount = -981.0f;

    // Applies gravity to an entity
    void ApplyGravity(Entity& entity);
    // Applies drag to an entity
    void ApplyDrag(Entity& entity);
    // Updates an entity's position and velocity over a fixed time step
    void IntegrateVelocity(Entity& entity, float fixedDeltaTime);
};

}

#endif
