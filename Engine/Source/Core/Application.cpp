#include "Application.h"
#include <iostream>

namespace RiverCore {

Application::Application() {
    // Initialize the internal window class object
    window = Window();

    // Initialize the input handler object
    input = Input();

    //Intialize the entities object
    entities = Entities();
}

Application::~Application() {

}

void Application::Init() {
    // Initialize SDL video subsystem
    if(!SDL_Init(SDL_INIT_VIDEO)){
        std::cout << "Error initializing SDL\n";
    }

    // Create the application window
    window.SetNativeWindow(
        SDL_CreateWindow(
            window.GetTitle().c_str(), 
            window.GetWidth(), 
            window.GetHeight(), 
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
        )
    );
    
    if(window.GetNativeWindow() == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL window: %s\n", SDL_GetError());
    }

    // Initialize renderer
    renderer.Init(window.GetNativeWindow());
}

void Application::Run() {
    // Initialize engine systems
    Init();

    // Core application loop
    bool done = false;
    while(!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        // Update input state
        SDL_PumpEvents();

        renderer.Clear();
        renderer.BeginFrame();
        SDL_RenderPresent(renderer.GetRenderer());
    }

    // Clean up engine systems
    SDL_DestroyWindow(window.GetNativeWindow());
    SDL_Quit();
}

}
