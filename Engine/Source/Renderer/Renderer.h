#ifndef RENDERER_H
#define RENDERER_H

#include "Entity.h"
#include <SDL3/SDL.h>
#include <vector>
#include <unordered_map>

namespace RiverCore {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Returns a pointer to the underlying SDL renderer
    SDL_Renderer* GetRenderer() const { return rendererRef; }

    // Initializes renderer
    void Init(SDL_Window* window);
    // Begins the render pass for the current frame
    void BeginFrame(float deltaTime);
    // Ends the render pass for the current frame
    void EndFrame();

    // Adds an entity to the render list given parameters and returns its ID
    uint32_t AddEntity(const char* spritePath, float Xpos = 0.0f, float Ypos = 0.0f, float rotation = 0.0f,
        float Xscale = 1.0f, float Yscale = 1.0f);
    // Adds an animated entity to the render list given parameters and returns its ID
    uint32_t AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos = 0.0f, float Ypos = 0.0f,
        float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f);
    // Removes an entity from the render list and index map given an ID
    void RemoveEntity(uint32_t entityID);
    // Removes all entities in the render list and index map
    void ClearEntities();
    // Returns the number of entities in the render list
    size_t GetEntityCount() const { return entities.size(); }
    // Updates an entity's position given an ID
    void UpdateEntityPosition(uint32_t entityID, float newX, float newY);
    // Flips an entity's sprite given an ID
    void FlipSprite(uint32_t entityID, bool flipX, bool flipY);

private:
    // Internal pointer to the underlying SDL renderer
    SDL_Renderer* rendererRef = nullptr;
    // Stores the width of the application window
    int windowWidth;
    // Stores the height of the application window
    int windowHeight;
    // Internal list of entities to render
    std::vector<Entity> entities;
    // Maps entity IDs to their index in the render list
    std::unordered_map<uint32_t, size_t> idToIndex;
    // ID for the next entity to be created
    uint32_t nextEntityID = 1;

    // Returns a pointer to an entity given an ID (internal use only)
    Entity* GetEntityByID(uint32_t ID);
};

}

#endif
