#ifndef ENTITIES_H
#define ENTITIES_H

#include "SDL3/SDL_render.h"

namespace RiverCore {

// Velocity struct for physics
struct Velocity {
    float x;
    float y;
    float mag;
};

// Data-only struct that defines variables for entities
struct Entity {
    uint32_t ID = 0;              // Internal identifier (default 0 for invalid entity)
    SDL_Texture* spriteSheet;     // Spritesheet to use for the entity sprite
    float spriteWidth;            // Width of sprite frame(s)
    float spriteHeight;           // Height of sprite frame(s)

    // Sprite animation
    int currentFrame = 0;         // Current animation frame
    int totalFrames = 1;          // Total frames in the animation
    float fps = 0.0f;             // Frames per second
    float elapsedTime = 0.0f;     // Time tracking for animations

    // Transform
    float Xpos = 0.0f;            // X position (default: 0.0)
    float Ypos = 0.0f;            // Y position (default: 0.0)
    float rotation = 0.0f;        // Rotation in degrees (default: 0.0)
    float Xscale = 1.0f;          // X scale (default: 1.0)
    float Yscale = 1.0f;          // Y scale (default: 1.0)

    //Velocity/Physics stuff
    Velocity velocity;
    bool physApplied;
};
    
}

#endif