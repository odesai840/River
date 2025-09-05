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
    entity.Xpos = Xpos;
    entity.Ypos = Ypos;
    entity.rotation = rotation;
    entity.Xscale = Xscale;
    entity.Yscale = Yscale;
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
    entity.Xpos = Xpos;
    entity.Ypos = Ypos;
    entity.rotation = rotation;
    entity.Xscale = Xscale;
    entity.Yscale = Yscale;
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
        entity->Xpos = newX;
        entity->Ypos = newY;

        // Clear the entity's collisions
        entity->collisions.clear();

        // Get the entity's rectangle
        float x1, x2, y1, y2;
        if (entity->Xscale >= 0) {
            x1 = entity->Xpos;
            x2 = entity->Xpos + entity->spriteWidth * entity->Xscale;
        } else {
            x2 = entity->Xpos;
            x1 = entity->Xpos + entity->spriteWidth * entity->Xscale;
        }
        if (entity->Yscale >= 0) {
            y1 = entity->Ypos;
            y2 = entity->Ypos + entity->spriteHeight * entity->Yscale;
        } else {
            y2 = entity->Ypos;
            y1 = entity->Ypos + entity->spriteHeight * entity->Yscale;
        }
        SDL_Rect entityRect = SDL_Rect{(int) x1, (int) y1, (int) (x2 - x1), (int) (y2 - y1)};

        // For each entity in the render list:
        for (Entity other : GetEntities()) {
            // If the entity is not this entity:
            if (other.ID != entityID) { 
                // Get the other entity's rectangle
                float ox1, ox2, oy1, oy2;
                if (other.Xscale >= 0) {
                    ox1 = other.Xpos;
                    ox2 = other.Xpos + other.spriteWidth * other.Xscale;
                } else {
                    ox2 = other.Xpos;
                    ox1 = other.Xpos + other.spriteWidth * other.Xscale;
                }
                if (entity->Yscale >= 0) {
                    oy1 = other.Ypos;
                    oy2 = other.Ypos + other.spriteHeight * other.Yscale;
                } else {
                    oy2 = other.Ypos;
                    oy1 = other.Ypos + other.spriteHeight * other.Yscale;
                }
                SDL_Rect otherRect = SDL_Rect{(int) ox1, (int) oy1, (int) (ox2 - ox1), (int) (oy2 - oy1)};

                // If the two rectangles intersect:
                if (SDL_HasRectIntersection(&entityRect, &otherRect)) {
                    // If the other entity interects this one from above:
                    if (oy2 >= y1 && oy2 <= y2) {
                        entity->collisions.push_back(std::tuple<Entity, int>(other, 0));
                    }

                    // If the other entity interects this one from the right:
                    if (ox1 >= x1 && ox1 <= x2) {
                        entity->collisions.push_back(std::tuple<Entity, int>(other, 1));
                    }

                    // If the other entity interects this one from below:
                    if (oy1 >= y1 && oy1 <= y2) {
                        entity->collisions.push_back(std::tuple<Entity, int>(other, 2));
                    }

                    // If the other entity interects this one from the left:
                    if (ox2 >= x1 && ox2 <= x2) {
                        entity->collisions.push_back(std::tuple<Entity, int>(other, 3));
                    }

                }
            }
        }
    }
}

void Renderer::FlipSprite(uint32_t entityID, bool flipX, bool flipY) {
    // Get the entity using its ID
    Entity* entity = GetEntityByID(entityID);
    // Flip its sprite by negating its scale
    if(entity && flipX) {
        entity->Xscale *= -1;
    }
    if(entity && flipY) {
        entity->Yscale *= -1;
    }
}

Entity* Renderer::GetEntityByID(uint32_t entityID) {
    // Find the entity with the matching ID in the index map
    auto it = idToIndex.find(entityID);
    // Return a reference to it if found
    return (it != idToIndex.end()) ? &entities[it->second] : nullptr;
}

}
