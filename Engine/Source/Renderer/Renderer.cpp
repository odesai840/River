#include "Renderer.h"
#include "Core/Window.h"
#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

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

void Renderer::BeginFrame(float deltaTime) {
    // Initialize the window width and height for scaling purposes
    SDL_GetRenderOutputSize(rendererRef, &windowWidth, &windowHeight);

    // Clear the render target with a dark blue color
    SDL_SetRenderDrawColor(rendererRef, 0x00, 0x00, 0x1F, 0xFF);
    SDL_RenderClear(rendererRef);

    // Calculate scaling factors based on the current scaling mode
    float globalScaleX, globalScaleY;
    CalculateScalingFactors(globalScaleX, globalScaleY);

    // Render all entities
    for (auto& entity : entities) {
        if (entity.spriteSheet != nullptr) {
            float spriteWidth = entity.spriteWidth;

            // Handle switching frames for animated entities
            if(entity.totalFrames > 1) {
                spriteWidth = entity.spriteWidth / static_cast<float>(entity.totalFrames);

                entity.elapsedTime += deltaTime;

                float frameTime = 1.0f / entity.fps;
                
                if (entity.elapsedTime >= frameTime) {
                    entity.currentFrame = (entity.currentFrame + 1) % entity.totalFrames;
                    entity.elapsedTime = 0.0f;
                }
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
            
            bool success = SDL_RenderTextureRotated(rendererRef, entity.spriteSheet, &srcRect, &dstRect,
                                                    entity.rotation, nullptr, SDL_FLIP_NONE);

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
                float globalScaleX, globalScaleY;
                CalculateScalingFactors(globalScaleX, globalScaleY);

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
    }
}

void Renderer::EndFrame() {
    SDL_RenderPresent(rendererRef);
}

uint32_t Renderer::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled) {
    // Load an image for the entity sprite
    SDL_Surface *spriteSheet = IMG_Load(spritePath);

    // Check if the image was loaded successfully
    if (!spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image %s: %s", spritePath, SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Create the entity and set its properties
    Entity entity;
    entity.ID = nextEntityID++;
    entity.spriteSheet = SDL_CreateTextureFromSurface(rendererRef, spriteSheet);
    entity.spriteWidth = static_cast<float>(spriteSheet->w);
    entity.spriteHeight = static_cast<float>(spriteSheet->h);
    entity.position.x = Xpos;
    entity.position.y = Ypos;
    entity.rotation = rotation;
    entity.scale.x = Xscale;
    entity.scale.y = Yscale;
    entity.physApplied = physEnabled;

    // Free the image surface after creating the texture
    SDL_DestroySurface(spriteSheet);

    // Check if the texture was created successfully
    if (!entity.spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s", SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Store the index to map the entity to
    size_t index = entities.size();
    // Add the entity to the render list
    entities.push_back(entity);
    // Add the entity to the index map
    idToIndex[entity.ID] = index;
    // Return the entity's ID
    return entity.ID;
}

uint32_t Renderer::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos, float Ypos,
    float rotation, float Xscale, float Yscale, bool physEnabled)
{
    // Load an image for the entity sprite
    SDL_Surface *spriteSheet = IMG_Load(spritePath);

    // Check if the image was loaded successfully
    if (!spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image %s: %s", spritePath, SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Create the entity and set its properties
    Entity entity;
    entity.ID = nextEntityID++;
    entity.spriteSheet = SDL_CreateTextureFromSurface(rendererRef, spriteSheet);
    entity.spriteWidth = static_cast<float>(spriteSheet->w);
    entity.spriteHeight = static_cast<float>(spriteSheet->h);
    entity.position.x = Xpos;
    entity.position.y = Ypos;
    entity.rotation = rotation;
    entity.scale.x = Xscale;
    entity.scale.y = Yscale;
    entity.totalFrames = totalFrames;
    entity.physApplied = physEnabled;
    entity.fps = fps;

    // Free the image surface after creating the texture
    SDL_DestroySurface(spriteSheet);

    if (!entity.spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s", SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Store the index to map the entity to
    size_t index = entities.size();
    // Add the entity to the render list
    entities.push_back(entity);
    // Add the entity to the index map
    idToIndex[entity.ID] = index;
    // Return the entity's ID
    return entity.ID;
}

void Renderer::RemoveEntity(uint32_t entityID) {
    // Find the entity with the matching ID in the index map
    auto it = idToIndex.find(entityID);
    // Early return if ID is not found
    if (it == idToIndex.end()) return;

    size_t index = it->second;
    SDL_DestroyTexture(entities[index].spriteSheet);

    // Swap with the last element and pop
    if (index != entities.size() - 1) {
        entities[index] = entities.back();
        // Update swapped entity's index
        idToIndex[entities[index].ID] = index;
    }
    entities.pop_back();

    // Remove the entity from the index map
    idToIndex.erase(it); 
}

void Renderer::ClearEntities() {
    // Delete all entity sprites and clear the render list and index map
    for (auto& entity : entities) {
        SDL_DestroyTexture(entity.spriteSheet);
    }
    entities.clear();
    idToIndex.clear();
}

void Renderer::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    // Get the entity using its ID
    Entity* entity = GetEntityByID(entityID);
    // If found:
    if (entity) {
        //Update the entity's position
        entity->position.x = newX;
        entity->position.y = newY;
    }
}

void Renderer::FlipSprite(uint32_t entityID, bool flipX, bool flipY) {
    // Get the entity using its ID
    Entity* entity = GetEntityByID(entityID);
    // Flip its sprite by negating its scale
    if(entity && flipX) {
        entity->scale.x *= -1;
    }
    if(entity && flipY) {
        entity->scale.y *= -1;
    }
}

Entity* Renderer::GetEntityByID(uint32_t entityID) {
    // Find the entity with the matching ID in the index map
    auto it = idToIndex.find(entityID);
    // Return a reference to it if found
    return (it != idToIndex.end()) ? &entities[it->second] : nullptr;
}

void Renderer::SetPosition(Entity& entity, const Vec2& position) {
    entity.position = position;
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
