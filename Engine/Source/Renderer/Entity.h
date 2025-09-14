#ifndef ENTITIES_H
#define ENTITIES_H

#include "Math/Math.h"
#include "SDL3/SDL_render.h"
#include <vector>

namespace RiverCore {

// Enum for collider types
enum class ColliderType {
    NONE,     // No collision detection
    SOLID,    // Full physics collision with position resolution
    TRIGGER   // Detection only, no position resolution
};

// Struct to define entity collider properties
struct Collider {
    ColliderType type = ColliderType::SOLID;

    // Collision properties
    bool enabled = true;
    Vec2 offset = Vec2::zero();        // Offset from the center of the entity
    Vec2 size = Vec2::zero();          // Custom size (0,0 = use sprite size)

    // Add a collision record to the collision vector for this entity
    void AddCollision(uint32_t entityID, int side);
    // Check if this entity is colliding with another entity
    bool IsCollidingWith(uint32_t entityID) const;
    // Check if this entity has a collision on a specific side
    bool HasCollisionOnSide(int side) const;
    // Clear all collision records for this entity
    void ClearCollisions();
    // Get all collision records for this entity
    const std::vector<std::pair<uint32_t, int>>& GetCollisions() const;

private:
    // Vector to store collision records for this entity
    std::vector<std::pair<uint32_t, int>> collisions;
};

// Data-only struct that defines variables for entities
struct Entity {
    uint32_t ID = 0;                   // Internal identifier (default 0 for invalid entity)
    SDL_Texture* spriteSheet;          // Spritesheet to use for the entity sprite
    float spriteWidth;                 // Width of sprite frame(s)
    float spriteHeight;                // Height of sprite frame(s)

    // Sprite animation
    int currentFrame = 0;              // Current animation frame
    int totalFrames = 1;               // Total frames in the animation
    float fps = 0.0f;                  // Frames per second
    float elapsedTime = 0.0f;          // Time tracking for animations

    // Transform
    Vec2 position = Vec2::zero();      // Position (default: Vec2::zero())
    float rotation = 0.0f;             // Rotation in degrees (default: 0.0)
    Vec2 scale = Vec2::one();          // Scale (default: Vec2::one())
    bool flipX = false;                // Horizontal flip
    bool flipY = false;                // Vertical flip

    // Physics
    Vec2 velocity = Vec2::zero();      // Velocity vector
    Vec2 acceleration = Vec2::zero();  // Acceleration vector
    bool physApplied = false;          // Whether physics is applied to this entity
    float mass = 1.0f;                 // Mass for physics calculations
    float drag = 0.0f;                 // Air resistance/drag coefficient

    // Collision
    Collider collider;                 // The collider for this entity
};

}

#endif