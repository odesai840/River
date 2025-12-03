#include "EntityManager.h"
#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

namespace RiverCore {

EntityManager::EntityManager() {

}

EntityManager::~EntityManager() {
    std::lock_guard<std::mutex> lock(entityMutex);
    // Clean up any loaded textures
    for (auto& entity : entities) {
        if (entity.spriteSheet != nullptr) {
            SDL_DestroyTexture(entity.spriteSheet);
        }
    }
}

uint32_t EntityManager::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation,
    float Xscale, float Yscale, bool physEnabled)
{
    // Input validation
    if (!spritePath || spritePath[0] == '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AddEntity: Invalid sprite path");
        return 0;
    }

    std::lock_guard<std::mutex> lock(entityMutex);

    // Load texture and get dimensions
    TextureInfo textureInfo = LoadTexture(spritePath);
    // Check if loading failed (null texture AND zero dimensions)
    // In headless mode, texture will be null but dimensions will be valid
    if (!textureInfo.texture && textureInfo.width == 0.0f && textureInfo.height == 0.0f) {
        return 0; // Failed to load texture/dimensions
    }

    Entity newEntity;
    newEntity.ID = nextEntityID++;
    newEntity.spritePath = spritePath;  // Store for replication
    newEntity.spriteSheet = textureInfo.texture;
    newEntity.spriteWidth = textureInfo.width;
    newEntity.spriteHeight = textureInfo.height;
    newEntity.position = Vec2(Xpos, Ypos);
    newEntity.rotation = rotation;
    newEntity.scale = Vec2(Xscale, Yscale);
    newEntity.physApplied = physEnabled;

    // Add to the entity vector and update the index map
    entities.push_back(newEntity);
    idToIndex[newEntity.ID] = entities.size() - 1;

    return newEntity.ID;
}

uint32_t EntityManager::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps,
    float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled)
{

    // Input validation
    if (!spritePath || spritePath[0] == '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AddAnimatedEntity: Invalid sprite path");
        return 0;
    }
    if (totalFrames <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AddAnimatedEntity: Invalid totalFrames value: %d", totalFrames);
        return 0;
    }
    if (fps <= 0.0f) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AddAnimatedEntity: Invalid fps value: %f", fps);
        return 0;
    }

    std::lock_guard<std::mutex> lock(entityMutex);

    // Load texture and get dimensions
    TextureInfo textureInfo = LoadTexture(spritePath);
    // Check if loading failed (null texture AND zero dimensions)
    // In headless mode, texture will be null but dimensions will be valid
    if (!textureInfo.texture && textureInfo.width == 0.0f && textureInfo.height == 0.0f) {
        return 0; // Failed to load texture/dimensions
    }

    Entity newEntity;
    newEntity.ID = nextEntityID++;
    newEntity.spritePath = spritePath;  // Store for replication
    newEntity.spriteSheet = textureInfo.texture;
    newEntity.spriteWidth = textureInfo.width;
    newEntity.spriteHeight = textureInfo.height;
    newEntity.position = Vec2(Xpos, Ypos);
    newEntity.rotation = rotation;
    newEntity.scale = Vec2(Xscale, Yscale);
    newEntity.physApplied = physEnabled;

    // Animation properties
    newEntity.totalFrames = totalFrames;
    newEntity.fps = fps;
    newEntity.currentFrame = 0;
    newEntity.elapsedTime = 0.0f;

    // Add to the entity vector and update the index map
    entities.push_back(newEntity);
    idToIndex[newEntity.ID] = entities.size() - 1;

    return newEntity.ID;
}

uint32_t EntityManager::AddSpritelessEntity(float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
    float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled)
{
    std::lock_guard<std::mutex> lock(entityMutex);

    Entity newEntity;
    newEntity.ID = nextEntityID++;
    newEntity.isSpriteless = true;
    newEntity.spritelessWidth = width;
    newEntity.spritelessHeight = height;
    newEntity.spritelessR = r;
    newEntity.spritelessG = g;
    newEntity.spritelessB = b;
    newEntity.spritelessA = a;
    newEntity.position = Vec2(Xpos, Ypos);
    newEntity.rotation = rotation;
    newEntity.scale = Vec2(Xscale, Yscale);
    newEntity.physApplied = physEnabled;

    // Set collider size
    newEntity.collider.size = Vec2(width * Xscale, height * Yscale);
    newEntity.collider.type = ColliderType::SOLID;

    // Set texture to empty
    newEntity.spriteSheet = nullptr;
    newEntity.spritePath = "";
    newEntity.spriteWidth = width;
    newEntity.spriteHeight = height;

    // Add to the entity vector and update the index map
    entities.push_back(newEntity);
    idToIndex[newEntity.ID] = entities.size() - 1;

    return newEntity.ID;
}

void EntityManager::RemoveEntity(uint32_t entityID) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it == idToIndex.end()) {
        return; // Entity not found
    }

    size_t index = it->second;

    // Clean up texture if it exists
    if (entities[index].spriteSheet != nullptr) {
        SDL_DestroyTexture(entities[index].spriteSheet);
    }

    // Remove from entity vector using swap-and-pop for efficiency
    if (index < entities.size() - 1) {
        std::swap(entities[index], entities.back());
    }
    entities.pop_back();

    // Remove from index map and update
    idToIndex.erase(it);
    UpdateIndexMap();
}

void EntityManager::ClearEntities() {
    std::lock_guard<std::mutex> lock(entityMutex);

    // Clean up all textures
    for (auto& entity : entities) {
        if (entity.spriteSheet != nullptr) {
            SDL_DestroyTexture(entity.spriteSheet);
        }
    }

    entities.clear();
    idToIndex.clear();
    nextEntityID = 1;
}

std::vector<Entity> EntityManager::GetEntitiesCopy() const {
    std::lock_guard<std::mutex> lock(entityMutex);
    return entities; // Copy the vector
}

Entity* EntityManager::GetEntityByID(uint32_t ID) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(ID);
    if (it == idToIndex.end()) {
        return nullptr;
    }

    return &entities[it->second];
}

size_t EntityManager::GetEntityCount() const {
    std::lock_guard<std::mutex> lock(entityMutex);
    return entities.size();
}

bool EntityManager::EntityExists(uint32_t ID) const {
    std::lock_guard<std::mutex> lock(entityMutex);
    return idToIndex.find(ID) != idToIndex.end();
}

bool EntityManager::GetEntityProperty(uint32_t ID, std::function<void(const Entity&)> accessor) const {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(ID);
    if (it == idToIndex.end()) {
        return false; // Entity not found
    }

    accessor(entities[it->second]);
    return true;
}

void EntityManager::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].position = Vec2(newX, newY);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "UpdateEntityPosition: Entity ID %u not found", entityID);
    }
}

void EntityManager::FlipSprite(uint32_t entityID, bool flipX, bool flipY) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].flipX = flipX;
        entities[it->second].flipY = flipY;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "FlipSprite: Entity ID %u not found", entityID);
    }
}

void EntityManager::SetPosition(uint32_t entityID, const Vec2& position) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].position = position;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SetPosition: Entity ID %u not found", entityID);
    }
}

bool EntityManager::GetFlipX(uint32_t entityID) const {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        return entities[it->second].flipX;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "GetFlipX: Entity ID %u not found", entityID);
        return false;
    }
}

bool EntityManager::GetFlipY(uint32_t entityID) const {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        return entities[it->second].flipY;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "GetFlipY: Entity ID %u not found", entityID);
        return false;
    }
}

bool EntityManager::GetFlipState(uint32_t entityID, bool& flipX, bool& flipY) const {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        flipX = entities[it->second].flipX;
        flipY = entities[it->second].flipY;
        return true;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "GetFlipState: Entity ID %u not found", entityID);
        flipX = false;
        flipY = false;
        return false;
    }
}

void EntityManager::ToggleFlipX(uint32_t entityID) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].flipX = !entities[it->second].flipX;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ToggleFlipX: Entity ID %u not found", entityID);
    }
}

void EntityManager::ToggleFlipY(uint32_t entityID) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].flipY = !entities[it->second].flipY;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ToggleFlipY: Entity ID %u not found", entityID);
    }
}

void EntityManager::SetColliderType(uint32_t entityID, ColliderType type) {
    std::lock_guard<std::mutex> lock(entityMutex);

    auto it = idToIndex.find(entityID);
    if (it != idToIndex.end()) {
        entities[it->second].collider.type = type;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SetColliderType: Entity ID %u not found", entityID);
    }
}

void EntityManager::UpdatePhysics(std::function<void(std::vector<Entity>&)> physicsUpdate) {
    // Copy entities to a new vector under a short lock
    std::vector<Entity> entitiesCopy;
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        entitiesCopy = entities;
    }

    // Work on the copy
    physicsUpdate(entitiesCopy);

    // Apply changes back under a short lock
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        // Copy physics results back (positions, velocities, collisions)
        for (size_t i = 0; i < entities.size() && i < entitiesCopy.size(); ++i) {
            entities[i].position = entitiesCopy[i].position;
            entities[i].velocity = entitiesCopy[i].velocity;
            entities[i].collider = entitiesCopy[i].collider;
        }
    }
}

void EntityManager::UpdateAnimations(float deltaTime) {
    std::lock_guard<std::mutex> lock(entityMutex);

    // Handle animation updates for entities that need it
    for (auto& entity : entities) {
        if (entity.totalFrames > 1) {
            entity.elapsedTime += deltaTime;
            float frameTime = 1.0f / entity.fps;

            while (entity.elapsedTime >= frameTime) {
                entity.currentFrame = (entity.currentFrame + 1) % entity.totalFrames;
                entity.elapsedTime -= frameTime;
            }
        }
    }
}

void EntityManager::UpdateIndexMap() {
    idToIndex.clear();
    for (size_t i = 0; i < entities.size(); ++i) {
        idToIndex[entities[i].ID] = i;
    }
}

TextureInfo EntityManager::LoadTexture(const char* spritePath) {
    TextureInfo result = {nullptr, 0.0f, 0.0f};

    // In headless mode, skip texture loading but still get dimensions
    if (headlessMode) {
        // Load image to get dimensions (needed for physics/collisions)
        SDL_Surface *spriteSheet = IMG_Load(spritePath);

        if (!spriteSheet) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image %s: %s", spritePath, SDL_GetError());
            return result;
        }

        // Get dimensions from the surface
        result.width = static_cast<float>(spriteSheet->w);
        result.height = static_cast<float>(spriteSheet->h);
        result.texture = nullptr;  // No texture in headless mode

        // Free the image surface
        SDL_DestroySurface(spriteSheet);

        return result;
    }

    // Normal mode: require renderer
    if (!rendererRef) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Renderer not set in EntityManager");
        return result;
    }

    // Load an image for the entity sprite
    SDL_Surface *spriteSheet = IMG_Load(spritePath);

    // Check if the image was loaded successfully
    if (!spriteSheet) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image %s: %s", spritePath, SDL_GetError());
        return result;
    }

    // Get dimensions from the surface
    result.width = static_cast<float>(spriteSheet->w);
    result.height = static_cast<float>(spriteSheet->h);

    // Create texture from the surface
    result.texture = SDL_CreateTextureFromSurface(rendererRef, spriteSheet);

    // Free the image surface after creating the texture
    SDL_DestroySurface(spriteSheet);

    if (!result.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture: %s", SDL_GetError());
        result.width = 0.0f;
        result.height = 0.0f;
        return result;
    }

    return result;
}

}