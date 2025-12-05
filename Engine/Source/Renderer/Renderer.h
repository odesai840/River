#ifndef RENDERER_H
#define RENDERER_H

#include "Entity.h"
#include "EntityManager.h"
#include "Camera.h"
#include <SDL3/SDL.h>

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
    void BeginFrame(float deltaTime, EntityManager& entityManager);
    // Ends the render pass for the current frame
    void EndFrame();

    // Function to toggle the scaling mode
    void ToggleScalingMode();
    // Function to toggle the collision debug boxes
    void ToggleDebugCollisions();

    // Resize the renderer and update camera viewport
    void Resize(int width, int height);

    // Get a reference to the camera
    Camera& GetCamera();

private:
    // Internal pointer to the underlying SDL renderer
    SDL_Renderer* rendererRef = nullptr;
    // Stores the width of the application window
    int windowWidth;
    // Stores the height of the application window
    int windowHeight;

    // Scaling mode
    ScalingMode scalingMode = ScalingMode::Proportional;
    static constexpr float baseWindowWidth = 1920.0f;
    static constexpr float baseWindowHeight = 1080.0f;
    // Function to calculate scaling factors for scaling modes
    void CalculateScalingFactors(float& scaleX, float& scaleY) const;

    // Boolean flag to toggle collision debug boxes
    bool debugCollisions = false;

    // Camera for viewport transforms
    Camera camera;

    // Function to render an entity
    void RenderEntity(const Entity& entity, float globalScaleX, float globalScaleY) const;
};

}

#endif
