#ifndef ENTITIES_H
#define ENTITIES_H

#include "Math/Math.h"
#include "SDL3/SDL_render.h"
#include <vector>

namespace RiverCore {

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

    // Physics
    Vec2 velocity = Vec2::zero();      // Velocity vector
    Vec2 acceleration = Vec2::zero();  // Acceleration vector
    bool physApplied = false;          // Whether physics is applied to this entity
    float mass = 1.0f;                 // Mass for physics calculations
    float drag = 0.0f;                 // Air resistance/drag coefficient

    // Collisions are stored as a vector of pairs,
    // where each pair holds the other entity's ID and the side being collided with
    // (0 = top, 1 = right, 2 = bottom, 3 = left)
    // Note that the same two entities can have multiple collisions (ex. top and left)
    std::vector<std::pair<uint32_t, int>> collisions = std::vector<std::pair<uint32_t, int>>();
};
}

#endif