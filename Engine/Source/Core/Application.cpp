#include "Application.h"
#include <iostream>

namespace RiverCore {

Application::Application() {
    window = Window();
}

Application::~Application() {

}

void Application::Init() {
    bool done = false;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        std::cout << "Error initializing SDL\n";
    }

    window.SetNativeWindow(
        SDL_CreateWindow(
            window.GetTitle().c_str(), 
            window.GetWidth(), 
            window.GetHeight(), 
            SDL_WINDOW_VULKAN
        )
    );

    if(window.GetNativeWindow() == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL window: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window.GetNativeWindow(), NULL);

    if (renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL renderer: %s\n", SDL_GetError());
    }

    while(!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x1F, 0xFF);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window.GetNativeWindow());
    SDL_Quit();
}

void Application::Run() {
    Init();


}

}
