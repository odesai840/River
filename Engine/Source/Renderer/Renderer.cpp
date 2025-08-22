#include "Renderer.h"
#include "Core/Window.h"
#include <SDL3/SDL_log.h>

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

void Renderer::Clear() {
    // Clear the render target with a dark blue color
    SDL_SetRenderDrawColor(rendererRef, 0x00, 0x00, 0x1F, 0xFF);
    SDL_RenderClear(rendererRef);
}

void Renderer::BeginFrame() {
    // Initialize the window width and height for scaling purposes
    SDL_GetRenderOutputSize(rendererRef, &windowWidth, &windowHeight);

    // Draw a 200x200 rectangle for testing purposes
    SDL_FRect rect;
    SDL_SetRenderDrawColor(rendererRef, 0, 0, 255, 255);
    rect.w = 200;
    rect.h = 200;
    rect.x = (windowWidth - rect.w) / 2.0f;
    rect.y = (windowHeight - rect.h) / 2.0f;
    SDL_RenderFillRect(rendererRef, &rect);
}

}
