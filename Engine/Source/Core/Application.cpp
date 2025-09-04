#include "Application.h"
#include <chrono>
#include <iostream>

namespace RiverCore {

Application::Application() {
    // Initialize the internal window class object
    window = Window();

    // Initialize the input handler object
    input = Input();

    // Initalize the physics class object
    physics = Physics();
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
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
        )
    );
    
    if(window.GetNativeWindow() == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL window: %s\n", SDL_GetError());
    }

    // Initialize renderer
    renderer.Init(window.GetNativeWindow());
}

void Application::Run(GameInterface* game) {
    // Initialize engine systems
    Init();

    game->setPhysicsRef(&physics);
    game->SetRenderer(&renderer);
    game->SetInput(&input);
    
    game->OnStart();

    // Core application loop
    bool done = false;
    auto lastTime = std::chrono::high_resolution_clock::now();
    while(!done) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        // Update input state
        SDL_PumpEvents();
        
        game->OnUpdate(deltaTime);
        
        renderer.BeginFrame(deltaTime);
        renderer.EndFrame();
    }

    // Clean up engine systems
    SDL_DestroyWindow(window.GetNativeWindow());
    SDL_Quit();
}

}
