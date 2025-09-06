#ifndef RENDERER_H
#define RENDERER_H

#include "Entity.h"
#include <SDL3/SDL.h>
#include <vector>
#include <unordered_map>

namespace RiverCore {

// Enum for different scaling modes
enum class ScalingMode {
    PixelBased,    // Constant size (pixel-based)
    Proportional   // Proportional scaling (percentage-based)
};

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
        float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Adds an animated entity to the render list given parameters and returns its ID
    uint32_t AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos = 0.0f, float Ypos = 0.0f,
        float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Removes an entity from the render list and index map given an ID
    void RemoveEntity(uint32_t entityID);
    // Removes all entities in the render list and index map
    void ClearEntities();
    // Returns the list of entities being rendered
    std::vector<Entity> GetEntities() const { return entities; }
    // Returns the number of entities in the render list
    size_t GetEntityCount() const { return entities.size(); }
    // Updates an entity's position given an ID
    void UpdateEntityPosition(uint32_t entityID, float newX, float newY);
    // Updates an entity's collisions given an ID
    void UpdateEntityCollisions(uint32_t entityID);
    // Flips an entity's sprite given an ID
    void FlipSprite(uint32_t entityID, bool flipX, bool flipY);
    // Returns a pointer to an entity given an ID
    Entity* GetEntityByID(uint32_t ID);
    
    void ToggleScalingMode();

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

    ScalingMode scalingMode = ScalingMode::Proportional;
    float baseWindowWidth = 1920.0f;
    float baseWindowHeight = 1080.0f;
    void CalculateScalingFactors(float& scaleX, float& scaleY) const;
};

}

#endif
