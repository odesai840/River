#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "Entity.h"
#include "Math/Math.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <SDL3/SDL.h>

namespace RiverCore {

// Struct to hold texture and its dimensions
struct TextureInfo {
    SDL_Texture* texture;
    float width;
    float height;
};

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    // Set renderer for texture loading
    void SetRenderer(SDL_Renderer* renderer) { rendererRef = renderer; }

    // Thread-safe function to add an entity
    uint32_t AddEntity(const char* spritePath, float Xpos = 0.0f, float Ypos = 0.0f, float rotation = 0.0f,
        float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Thread-safe function to add an animated entity
    uint32_t AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos = 0.0f, float Ypos = 0.0f,
        float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Thread-safe function to remove an entity
    void RemoveEntity(uint32_t entityID);
    // Thread-safe function to clear all entities
    void ClearEntities();

    // Thread-safe function to get a copy of all entities
    std::vector<Entity> GetEntitiesCopy() const;
    // Thread-safe function to get a pointer to an entity using an ID
    Entity* GetEntityByID(uint32_t ID);
    // Thread-safe function to get the current entity count
    size_t GetEntityCount() const;

    // Thread-safe function to check if an entity exists
    bool EntityExists(uint32_t ID) const;
    // Thread-safe function to get a property of an entity
    bool GetEntityProperty(uint32_t ID, std::function<void(const Entity&)> accessor) const;

    // Thread-safe function to update an entity's position
    void UpdateEntityPosition(uint32_t entityID, float newX, float newY);
    // Thread-safe function to flip an entity's sprite
    void FlipSprite(uint32_t entityID, bool flipX, bool flipY);
    // Thread-safe function to set an entity's position
    void SetPosition(uint32_t entityID, const Vec2& position);

    // Thread-safe function to get the X-axis flip state of an entity
    bool GetFlipX(uint32_t entityID) const;
    // Thread-safe function to get the Y-axis flip state of an entity
    bool GetFlipY(uint32_t entityID) const;
    // Thread-safe function to get the flip state of an entity
    bool GetFlipState(uint32_t entityID, bool& flipX, bool& flipY) const;

    // Thread-safe function to toggle the X-axis flip state of an entity
    void ToggleFlipX(uint32_t entityID);
    // Thread-safe function to toggle the Y-axis flip state of an entity
    void ToggleFlipY(uint32_t entityID);

    // Thread-safe function to set the collider type of an entity
    void SetColliderType(uint32_t entityID, ColliderType type);

    // Thread-safe function to update the physics of all entities
    void UpdatePhysics(std::function<void(std::vector<Entity>&)> physicsUpdate);

    // Thread-safe function to update the animations of all entities
    void UpdateAnimations(float deltaTime);

    // Function to get the mutex for thread-safe operations
    std::mutex& GetMutex() { return entityMutex; }
    // Function to get the entity vector for thread-safe operations
    std::vector<Entity>& GetEntitiesUnsafe() { return entities; }

private:
    // Mutex for thread-safe operations
    mutable std::mutex entityMutex;
    // Vector of entities
    std::vector<Entity> entities;
    // Map of entity IDs to indices in the vector
    std::unordered_map<uint32_t, size_t> idToIndex;
    // Next available entity ID
    uint32_t nextEntityID = 1;
    // Reference to the SDL renderer
    SDL_Renderer* rendererRef = nullptr;

    // Function to load a texture from a file path
    TextureInfo LoadTexture(const char* spritePath);
    // Function to update the index map for entity IDs
    void UpdateIndexMap();
};

}

#endif