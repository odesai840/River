#include "Application.h"
#include <chrono>
#include <iostream>

namespace RiverCore {

Application::Application() {
    // Initialize the internal window class object
    window = Window();

    // Initialize the input handler object
    input = Input();

    // Initialize the physics class object
    physics = Physics();
}

Application::~Application() {
    // Ensure threads are properly stopped
    running = false;
    renderCondition.notify_all();

    if (physicsThread.joinable()) {
        physicsThread.join();
    }
    if (renderThread.joinable()) {
        renderThread.join();
    }
    if (networkThread.joinable()) {
        networkThread.join();
    }
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
    // Initialize entity manager
    entityManager.SetRenderer(renderer.GetRenderer());
}

void Application::PhysicsThreadFunction() {
    while (running) {
        // Update physics system
        entityManager.UpdatePhysics([this](std::vector<Entity>& entities) {
            float effectiveTimestep = timeline.CalculateEffectiveTime(FIXED_TIMESTEP);
            physics.UpdatePhysics(entities, effectiveTimestep);
        });

        // Sleep to maintain 60 Hz update rate
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Application::RenderThreadFunction() {
    // Last frame time
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running) {
        // Wait for a render signal from the main thread
        std::unique_lock<std::mutex> lock(renderMutex);
        renderCondition.wait(lock, [this] { return renderReady.load() || !running.load(); });

        // Stop render thread if the application was just stopped
        if (!running) {
            break;
        }

        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        deltaTime = std::min(deltaTime, MAX_FRAME_TIME);

        float effectiveDeltaTime = timeline.CalculateEffectiveTime(deltaTime);

        // Reset render signal
        renderReady = false;
        // Release lock allow rendering events
        lock.unlock();

        // Update animations
        entityManager.UpdateAnimations(effectiveDeltaTime);

        // Update game logic
        gameRef->OnUpdate(effectiveDeltaTime);

        // Render the frame
        renderer.BeginFrame(effectiveDeltaTime, entityManager);
        renderer.EndFrame();
    }
}

void Application::NetworkThreadFunction() {
    while (running) {
        // Update networking system
        networkManager.Update();

        // Sleep to maintain 60 Hz update rate
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Application::Run(GameInterface* game) {
    // Initialize engine systems
    Init();

    // Initialize game interface reference
    gameRef = game;
    
    // Set up game references
    game->SetPhysicsRef(&physics);
    game->SetRenderer(&renderer);
    game->SetInput(&input);
    game->SetEntityManager(&entityManager);
    game->SetTimeline(&timeline);

    // Run game start method
    game->OnStart();

    // Start worker threads
    physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
    renderThread = std::thread(&Application::RenderThreadFunction, this);

    // Main update loop
    bool done = false;
    while (!done && running) {
        // Handle SDL quit event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        // Update input state
        SDL_PumpEvents();

        // Signal render thread to render this frame
        {
            std::lock_guard<std::mutex> lock(renderMutex);
            renderReady = true;
        }
        renderCondition.notify_one();
    }

    // Signal threads to stop
    running = false;
    renderCondition.notify_all();

    // Wait for threads to finish
    if (physicsThread.joinable()) {
        physicsThread.join();
    }
    if (renderThread.joinable()) {
        renderThread.join();
    }

    // Clean up SDL resources
    SDL_DestroyRenderer(renderer.GetRenderer());
    SDL_DestroyWindow(window.GetNativeWindow());
    SDL_Quit();
}

void Application::RunServer() {
    // Initialize server
    Server server;
    server.Start();

    std::cout << "Headless server started successfully. Press Ctrl+C to stop.\n";

    // Keep server running until interrupted
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Application::RunClient(const std::string& serverAddress, GameInterface* game) {
    // Initialize engine systems
    Init();

    // Initialize game interface reference
    gameRef = game;

    // Set up game references
    game->SetPhysicsRef(&physics);
    game->SetRenderer(&renderer);
    game->SetInput(&input);
    game->SetEntityManager(&entityManager);
    game->SetTimeline(&timeline);

    // Initialize NetworkManager and connect to server
    networkManager.SetEntityManager(&entityManager);
    if (!networkManager.Connect(serverAddress)) {
        std::cout << "Failed to connect to server at " << serverAddress << std::endl;
        return;
    }

    // Set network manager reference for game
    game->SetNetworkManager(&networkManager);

    // Run game start method
    game->OnStart();

    // Start worker threads
    physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
    renderThread = std::thread(&Application::RenderThreadFunction, this);
    networkThread = std::thread(&Application::NetworkThreadFunction, this);

    // Main update loop
    bool done = false;
    while (!done && running) {
        // Handle SDL quit event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        // Update input state
        SDL_PumpEvents();

        // Signal render thread to render this frame
        {
            std::lock_guard<std::mutex> lock(renderMutex);
            renderReady = true;
        }
        renderCondition.notify_one();
    }

    // Disconnect from server
    networkManager.Disconnect();

    // Signal threads to stop
    running = false;
    renderCondition.notify_all();

    // Wait for threads to finish
    if (physicsThread.joinable()) {
        physicsThread.join();
    }
    if (renderThread.joinable()) {
        renderThread.join();
    }
    if (networkThread.joinable()) {
        networkThread.join();
    }

    // Clean up SDL resources
    SDL_DestroyRenderer(renderer.GetRenderer());
    SDL_DestroyWindow(window.GetNativeWindow());
    SDL_Quit();
}


}
