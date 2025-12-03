#include "Renderer.h"
#include <SDL3/SDL_log.h>

namespace RiverCore {

Renderer::Renderer()
    : camera(1920, 1080)
{

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

    // Update camera viewport to match window size
    camera.SetViewportSize(windowWidth, windowHeight);
}

void Renderer::BeginFrame(float deltaTime, EntityManager& entityManager) {
    // Initialize the window width and height for scaling purposes
    int newWidth, newHeight;
    SDL_GetRenderOutputSize(rendererRef, &newWidth, &newHeight);

    // Update camera viewport if window size changed
    if (newWidth != windowWidth || newHeight != windowHeight) {
        windowWidth = newWidth;
        windowHeight = newHeight;
        camera.SetViewportSize(windowWidth, windowHeight);
    }

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
    // Handle spriteless entities
    if (entity.isSpriteless) {
        // Calculate scaled dimensions
        float scaledWidth = entity.spritelessWidth * entity.scale.x * globalScaleX;
        float scaledHeight = entity.spritelessHeight * entity.scale.y * globalScaleY;

        // Apply camera transform to get camera-relative position
        Vec2 cameraRelativePos = camera.ApplyCameraTransform(entity.position);

        // Calculate screen position
        float finalXPos, finalYPos;
        if (scalingMode == ScalingMode::PixelBased) {
            finalXPos = (cameraRelativePos.x + (static_cast<float>(windowWidth) / 2.0f)) - (scaledWidth / 2.0f);
            finalYPos = (-cameraRelativePos.y + (static_cast<float>(windowHeight) / 2.0f)) - (scaledHeight / 2.0f);
        } else {
            float scaledXPos = cameraRelativePos.x * globalScaleX;
            float scaledYPos = cameraRelativePos.y * globalScaleY;
            finalXPos = (scaledXPos + (static_cast<float>(windowWidth) / 2.0f)) - (scaledWidth / 2.0f);
            finalYPos = (-scaledYPos + (static_cast<float>(windowHeight) / 2.0f)) - (scaledHeight / 2.0f);
        }

        // Create SDL rectangle
        SDL_FRect rect = {
            finalXPos,
            finalYPos,
            scaledWidth,
            scaledHeight
        };

        // Set draw color with alpha
        SDL_SetRenderDrawColor(rendererRef, entity.spritelessR, entity.spritelessG, entity.spritelessB, entity.spritelessA);
        SDL_SetRenderDrawBlendMode(rendererRef, SDL_BLENDMODE_BLEND);

        // Draw filled rectangle
        SDL_RenderFillRect(rendererRef, &rect);

        // Draw debug collision box if enabled
        if (debugCollisions && entity.collider.type != ColliderType::NONE) {
            SDL_SetRenderDrawColor(rendererRef, 255, 0, 0, 255);
            SDL_RenderRect(rendererRef, &rect);
        }

        return;
    }

    // Handle sprite entities
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

    // Apply camera transform to get camera-relative position (includes camera offset and zoom)
    Vec2 cameraRelativePos = camera.ApplyCameraTransform(entity.position);

    // Calculate sprite position with scaling mode consideration
    float finalXPos, finalYPos;
    if (scalingMode == ScalingMode::PixelBased) {
        // In pixel-based mode, positions remain constant in screen coordinates
        finalXPos = (cameraRelativePos.x + (static_cast<float>(windowWidth) / 2.0f)) - (finalSpriteWidth / 2.0f);
        finalYPos = (-cameraRelativePos.y + (static_cast<float>(windowHeight) / 2.0f)) - (finalSpriteHeight / 2.0f);
    } else {
        // In proportional mode, positions scale with the window
        float scaledXPos = cameraRelativePos.x * globalScaleX;
        float scaledYPos = cameraRelativePos.y * globalScaleY;
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
        Vec2 worldCollisionPos = Vec2(
            entity.position.x - (collisionWidth / 2.0f),
            entity.position.y - (collisionHeight / 2.0f)
        );

        // Apply camera transform
        Vec2 cameraRelativeCollisionPos = camera.ApplyCameraTransform(worldCollisionPos);

        // Convert camera-relative space to screen space
        float screenX, screenY;
        if (scalingMode == ScalingMode::PixelBased) {
            screenX = (cameraRelativeCollisionPos.x + (static_cast<float>(windowWidth) / 2.0f));
            screenY = (-cameraRelativeCollisionPos.y + (static_cast<float>(windowHeight) / 2.0f)) - collisionHeight;
        } else {
            float scaledX = cameraRelativeCollisionPos.x * globalScaleX;
            float scaledY = cameraRelativeCollisionPos.y * globalScaleY;
            screenX = (scaledX + (static_cast<float>(windowWidth) / 2.0f));
            screenY = (-scaledY + (static_cast<float>(windowHeight) / 2.0f)) - (collisionHeight * globalScaleY);
        }

        // Draw debug collision box with correct dimensions
        SDL_SetRenderDrawColor(rendererRef, 255, 0, 0, 255);
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

void Renderer::Resize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    camera.SetViewportSize(width, height);
}

Camera& Renderer::GetCamera() {
    return camera;
}

}
