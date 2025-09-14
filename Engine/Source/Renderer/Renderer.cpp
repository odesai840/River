#include "Renderer.h"
#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

namespace RiverCore {

Renderer::Renderer() {
    
}

Renderer::~Renderer() {
    
}

void Renderer::Init(SDL_Window* window) {
    // Initialize the SDL renderer
    rendererRef = SDL_CreateRenderer(window, NULL);

    if (rendererRef == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL renderer: %s\n", SDL_GetError());
    }

    // Get the initial window size for the base resolution
    SDL_GetRenderOutputSize(rendererRef, &windowWidth, &windowHeight);
    baseWindowWidth = static_cast<float>(windowWidth);
    baseWindowHeight = static_cast<float>(windowHeight);
}

void Renderer::BeginFrame(float deltaTime, EntityManager& entityManager) {
    // Initialize the window width and height for scaling purposes
    SDL_GetRenderOutputSize(rendererRef, &windowWidth, &windowHeight);

    // Clear the render target with a dark blue color
    SDL_SetRenderDrawColor(rendererRef, 0x00, 0x00, 0x1F, 0xFF);
    SDL_RenderClear(rendererRef);

    // Calculate scaling factors based on the current scaling mode
    float globalScaleX, globalScaleY;
    CalculateScalingFactors(globalScaleX, globalScaleY);

    // Get entities copy for rendering (thread-safe)
    std::vector<Entity> entities = entityManager.GetEntitiesCopy();

    // Render all entities
    for (const auto& entity : entities) {
        RenderEntity(entity, globalScaleX, globalScaleY);
    }
}

void Renderer::EndFrame() {
    SDL_RenderPresent(rendererRef);
}

void Renderer::RenderEntity(const Entity& entity, float globalScaleX, float globalScaleY) const {
    if (entity.spriteSheet == nullptr) return;

    float spriteWidth = entity.spriteWidth;

    // Handle switching frames for animated entities
    if(entity.totalFrames > 1) {
        spriteWidth = entity.spriteWidth / static_cast<float>(entity.totalFrames);
    }

    // Render the entity sprite to the screen
    SDL_FRect srcRect = {
        (static_cast<float>(entity.currentFrame) * spriteWidth),
        0.0f,
        spriteWidth,
        entity.spriteHeight
    };

    // Apply scaling mode calculations
    float finalSpriteWidth = spriteWidth * entity.scale.x * globalScaleX;
    float finalSpriteHeight = entity.spriteHeight * entity.scale.y * globalScaleY;

    // Calculate sprite position with scaling mode consideration
    float finalXPos, finalYPos;
    if (scalingMode == ScalingMode::PixelBased) {
        // In pixel-based mode, positions remain constant in screen coordinates
        finalXPos = (entity.position.x + (static_cast<float>(windowWidth) / 2.0f)) - (finalSpriteWidth / 2.0f);
        finalYPos = (-entity.position.y + (static_cast<float>(windowHeight) / 2.0f)) - (finalSpriteHeight / 2.0f);
    } else {
        // In proportional mode, positions scale with the window
        float scaledXPos = entity.position.x * globalScaleX;
        float scaledYPos = entity.position.y * globalScaleY;
        finalXPos = (scaledXPos + (static_cast<float>(windowWidth) / 2.0f)) - (finalSpriteWidth / 2.0f);
        finalYPos = (-scaledYPos + (static_cast<float>(windowHeight) / 2.0f)) - (finalSpriteHeight / 2.0f);
    }

    // Set the destination rectangle for sprite rendering
    SDL_FRect dstRect = {
        finalXPos,
        finalYPos,
        finalSpriteWidth,
        finalSpriteHeight
    };

    // Determine flip flags based on entity settings
    SDL_FlipMode flipMode = SDL_FLIP_NONE;
    if (entity.flipX && entity.flipY) {
        flipMode = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    } else if (entity.flipX) {
        flipMode = SDL_FLIP_HORIZONTAL;
    } else if (entity.flipY) {
        flipMode = SDL_FLIP_VERTICAL;
    }

    bool success = SDL_RenderTextureRotated(rendererRef, entity.spriteSheet, &srcRect, &dstRect,
                                            entity.rotation, nullptr, flipMode);

    if (!success) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error rendering entity: %s\n", SDL_GetError());
    }

    if (debugCollisions) {
        // Calculate collision box dimensions
        float frameWidth = entity.totalFrames > 1 ?
            (entity.spriteWidth / static_cast<float>(entity.totalFrames)) : entity.spriteWidth;
        float collisionWidth = frameWidth * abs(entity.scale.x);
        float collisionHeight = entity.spriteHeight * abs(entity.scale.y);

        // Calculate world space collision bounds
        float worldX1 = entity.position.x - (collisionWidth / 2.0f);
        float worldY1 = entity.position.y - (collisionHeight / 2.0f);

        // Convert world space to screen space
        float screenX, screenY;
        if (scalingMode == ScalingMode::PixelBased) {
            screenX = (worldX1 + (static_cast<float>(windowWidth) / 2.0f));
            screenY = (-worldY1 + (static_cast<float>(windowHeight) / 2.0f)) - collisionHeight;
        } else {
            float scaledWorldX = worldX1 * globalScaleX;
            float scaledWorldY = worldY1 * globalScaleY;
            screenX = (scaledWorldX + (static_cast<float>(windowWidth) / 2.0f));
            screenY = (-scaledWorldY + (static_cast<float>(windowHeight) / 2.0f)) - (collisionHeight * globalScaleY);
        }

        // Draw debug collision box with correct dimensions
        SDL_SetRenderDrawColor(rendererRef, 255, 0, 0, 128);
        SDL_FRect debugRect = {
            screenX,
            screenY,
            collisionWidth * (scalingMode == ScalingMode::Proportional ? globalScaleX : 1.0f),
            collisionHeight * (scalingMode == ScalingMode::Proportional ? globalScaleY : 1.0f)
        };
        SDL_RenderRect(rendererRef, &debugRect);
    }
}

void Renderer::ToggleScalingMode() {
    ScalingMode newMode = (scalingMode == ScalingMode::PixelBased) ?
                           ScalingMode::Proportional : ScalingMode::PixelBased;
    scalingMode = newMode;
}

void Renderer::CalculateScalingFactors(float& scaleX, float& scaleY) const {
    switch (scalingMode) {
        case ScalingMode::PixelBased:
            // Constant size - no scaling based on window size
            scaleX = 1.0f;
            scaleY = 1.0f;
            break;
        case ScalingMode::Proportional:
            // Proportional scaling based on window size change
            scaleX = static_cast<float>(windowWidth) / baseWindowWidth;
            scaleY = static_cast<float>(windowHeight) / baseWindowHeight;
            break;
    }
}

void Renderer::ToggleDebugCollisions() {
    debugCollisions = !debugCollisions;
}


}
