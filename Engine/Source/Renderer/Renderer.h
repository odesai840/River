#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>

namespace RiverCore {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Returns a pointer to the underlying SDL renderer
    SDL_Renderer* GetRenderer() const { return rendererRef; }

    // Initializes renderer
    void Init(SDL_Window* window);
    // Clears the render target in preparation for the next frame
    void Clear();
    // Begins the render pass for the current frame
    void BeginFrame();

private:
    // Internal pointer to the underlying SDL renderer
    SDL_Renderer* rendererRef = nullptr;
    // Stores the width of the application window
    int windowWidth;
    // Stores the height of the application window
    int windowHeight;
};

}

#endif
