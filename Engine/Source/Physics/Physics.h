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

    // Gravity
    void SetGravity(const float gravity) { gravityAmount = gravity; }
    float GetGravity() const { return gravityAmount; }
    Vec2 GetGravityVector() const { return Vec2(0.0f, gravityAmount); }

    // Physics simulation
    void UpdatePhysics(std::vector<Entity>& entities, float deltaTime);

    // Collision detection
    void UpdateCollisions(std::vector<Entity>& entities);
    bool CheckAABBCollision(const Entity& a, const Entity& b) const;

    // Physics utilities
    void ApplyForce(Entity& entity, const Vec2& force);
    void ApplyImpulse(Entity& entity, const Vec2& impulse);
    void SetVelocity(Entity& entity, const Vec2& velocity);

private:
    float gravityAmount = -981.0f;

    // Helper functions
    void ApplyGravity(Entity& entity, float deltaTime);
    void ApplyDrag(Entity& entity, float deltaTime);
    void IntegrateVelocity(Entity& entity, float deltaTime);
};

}

#endif
