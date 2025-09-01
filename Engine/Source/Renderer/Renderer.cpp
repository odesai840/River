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
}

void Renderer::BeginFrame(float deltaTime) {
    // Initialize the window width and height for scaling purposes
    SDL_GetRenderOutputSize(rendererRef, &windowWidth, &windowHeight);

    // Clear the render target with a dark blue color
    SDL_SetRenderDrawColor(rendererRef, 0x00, 0x00, 0x1F, 0xFF);
    SDL_RenderClear(rendererRef);

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

            SDL_FRect dstRect = {
                (entity.Xpos + (static_cast<float>(windowWidth) / 2.0f)) - (spriteWidth * entity.Xscale / 2.0f),
                (-entity.Ypos + (static_cast<float>(windowHeight) / 2.0f)) - (entity.spriteHeight * entity.Yscale / 2.0f),
                spriteWidth * entity.Xscale,
                entity.spriteHeight * entity.Yscale
            };
            
            bool success = SDL_RenderTextureRotated(rendererRef, entity.spriteSheet, &srcRect, &dstRect,
                                                    entity.rotation, nullptr, SDL_FLIP_NONE);

            if (!success) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error rendering entity: %s\n", SDL_GetError());
            }
        }
    }
}

void Renderer::EndFrame() {
    SDL_RenderPresent(rendererRef);
}

uint32_t Renderer::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation, float Xscale, float Yscale) {
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
    entity.Xpos = Xpos;
    entity.Ypos = Ypos;
    entity.rotation = rotation;
    entity.Xscale = Xscale;
    entity.Yscale = Yscale;

    // Free the image surface after creating the texture
    SDL_DestroySurface(spriteSheet);

    // Check if the texture was created successfully
    if (!entity.spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s", SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Add the entity to the render list
    entities.push_back(entity);
    // Return the entity's ID
    return entity.ID;
}

uint32_t Renderer::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos, float Ypos,
    float rotation, float Xscale, float Yscale)
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
    entity.Xpos = Xpos;
    entity.Ypos = Ypos;
    entity.rotation = rotation;
    entity.Xscale = Xscale;
    entity.Yscale = Yscale;
    entity.totalFrames = totalFrames;
    entity.fps = fps;

    // Free the image surface after creating the texture
    SDL_DestroySurface(spriteSheet);

    if (!entity.spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s", SDL_GetError());
        // Return 0 on failure to invalidate the entity
        return 0;
    }

    // Add the entity to the render list
    entities.push_back(entity);
    // Return the entity's ID
    return entity.ID;
}

void Renderer::RemoveEntity(uint32_t ID) {
    // Remove the entity that matches the given ID from the render list
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (it->ID == ID) {
            SDL_DestroyTexture(it->spriteSheet);
            entities.erase(it);
            return;
        }
    }
}

void Renderer::ClearEntities() {
    // Delete all entity sprites and clear the render list
    for (auto& entity : entities) {
        SDL_DestroyTexture(entity.spriteSheet);
    }
    entities.clear();
}

void Renderer::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    // Update the position of the entity with the given ID
    for (auto& entity : entities) {
        if (entity.ID == entityID) {
            entity.Xpos = newX;
            entity.Ypos = newY;
            return;
        }
    }
}

}
