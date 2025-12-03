#include "Application.h"
#include "Networking/Server.h"
#include <chrono>
#include <iostream>
#include <csignal>

namespace RiverCore {

std::atomic<bool>* g_running = nullptr;
    
Application::Application()
    : allocator(32, 200)
{
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
    // Initialize renderer
    renderer.Init(window.GetNativeWindow());
    // Initialize entity manager
    entityManager.SetRenderer(renderer.GetRenderer());

    if (gameRef) {
        gameRef->OnStart();
    }

    rendererInitialized.store(true);
    
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

        // Update timeline
        timeline.Update(deltaTime);

        // Update game logic
        gameRef->OnUpdate(effectiveDeltaTime);

        // Update replay manager
        replayManager.Update(eventManager, effectiveDeltaTime);

        // Render the frame
        renderer.BeginFrame(effectiveDeltaTime, entityManager);
        renderer.EndFrame();
    }
}

void Application::RenderThreadFunction_ListenServer(Server* server) {
    // Initialize renderer
    renderer.Init(window.GetNativeWindow());
    // Initialize entity manager
    entityManager.SetRenderer(renderer.GetRenderer());
    server->GetEntityManager().SetRenderer(renderer.GetRenderer());

    if (gameRef) {
        gameRef->OnStart();
    }

    rendererInitialized.store(true);
    
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

        // Render the frame
        renderer.BeginFrame(effectiveDeltaTime, server->GetEntityManager());
        renderer.EndFrame();
    }
}

void Application::NetworkThreadFunction() {
    while (running) {
        // Update networking system
        networkManager.Update();

        // Sleep duration scaled by timeline to simulate different update rates
        float baseIntervalMs = 16.0f;
        float timeScale = timeline.GetTimeScale();
        float adjustedInterval = baseIntervalMs / timeScale;

        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(adjustedInterval)));
    }
}

void Application::Run(GameInterface* game) {
    // Initialize engine systems
    Init();

    // Set mode
    currentMode = NetworkMode::STANDALONE;

    // Initialize game interface reference
    gameRef = game;

    // Set up game references
    game->SetPhysicsRef(&physics);
    game->SetRenderer(&renderer);
    game->SetInput(&input);
    game->SetEntityManager(&entityManager);
    game->SetTimeline(&timeline);
    game->SetEventManager(&eventManager);
    eventManager.SetTimeline(&timeline);
    game->SetMode(NetworkMode::STANDALONE);
    game->SetMemory(&allocator);

    // Set up replay manager
    replayManager.SetEntityManager(&entityManager);
    game->SetReplayManager(&replayManager);

    // Connect EventManager to ReplayManager for input recording
    eventManager.SetInputRecordingCallback([this](const EventData& data) {
        replayManager.RecordInput(data);
    });

    // Start worker threads
    physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
    renderThread = std::thread(&Application::RenderThreadFunction, this);

    while(!rendererInitialized.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Run game start method
    //game->OnStart();

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

void Application::RunServer(GameInterface* game, bool headless) {
    if (!game) {
        std::cout << "Error: Cannot start server without game logic\n";
        return;
    }

    currentMode = NetworkMode::SERVER;

    if (!headless) {
        // Initialize engine systems for listen-server mode
        Init();
    }

    std::cout << "Starting server in " << (headless ? "headless" : "listen-server") << " mode...\n";

    // Initialize server
    Server server;

    // Set up game references to server's systems
    game->SetEntityManager(&server.GetEntityManager());
    game->SetPhysicsRef(&server.GetPhysics());
    game->SetTimeline(&server.GetTimeline());
    game->SetInputManager(&server.GetInputManager());
    game->SetEventManager(&server.GetEventManager());
    game->SetMode(NetworkMode::SERVER);
    game->SetServerRef(&server);
    game->SetHeadlessServer(headless);
    game->SetMemory(&allocator);

    // Enable headless mode only for dedicated servers
    server.GetEntityManager().SetHeadlessMode(headless);

    // Set renderer for listen-server entity manager
    if (headless) {
        // Initialize game logic
        game->OnStart();

        std::cout << "Game initialized, starting server...\n";

        // Set up signal handler
        g_running = &running;
        (void)std::signal(SIGINT, ServerSignalHandler);

        // Start server (blocks in simulation loop)
        std::thread serverThread([&server, game]() {
            server.Start(game);
        });

        // Shutdown monitor thread - watches running flag and stops server
        std::thread shutdownMonitor([this, &server]() {
            while (running.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            // Running flag set to false, stop the server
            std::cout << "Shutdown requested, stopping server...\n";
            server.Stop();
        });

        // Wait for server thread to complete
        std::cout << "Headless server running. Press Ctrl+C to stop.\n";
        
        if (serverThread.joinable()) {
            serverThread.join();
        }

        if (shutdownMonitor.joinable()) {
            shutdownMonitor.join();
        }

        // Restore default signal handler
        (void)std::signal(SIGINT, SIG_DFL);
        g_running = nullptr;
    }
    else {
        // Listen-server with local rendering
        gameRef = game;
        game->SetRenderer(&renderer);

        // Set input for listen-server
        game->SetInput(&input);

        // Start render thread with server's EntityManager
        renderThread = std::thread(&Application::RenderThreadFunction_ListenServer, this, &server);

        while (!rendererInitialized.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Initialize game logic
        //game->OnStart();

        // Start server (blocks in simulation loop)
        std::thread serverThread([&server, game]() {
            server.Start(game);
        });

        // Main event loop
        bool done = false;
        while (!done && running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    done = true;
                }
            }

            SDL_PumpEvents();

            // Signal render thread
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                renderReady = true;
            }
            renderCondition.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        running = false;
        renderCondition.notify_all();

        if (renderThread.joinable()) {
            renderThread.join();
        }

        // Stop server and wait for server thread to finish
        server.Stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }

        SDL_DestroyRenderer(renderer.GetRenderer());
        SDL_DestroyWindow(window.GetNativeWindow());
        SDL_Quit();
    }
}

void Application::RunClient(const std::string& serverAddress, GameInterface* game) {
    // Initialize engine systems
    Init();

    // Set mode
    currentMode = NetworkMode::CLIENT;

    // Initialize game interface reference
    gameRef = game;

    // Set up game references
    game->SetPhysicsRef(&physics);
    game->SetRenderer(&renderer);
    game->SetInput(&input);
    game->SetEntityManager(&entityManager);
    game->SetTimeline(&timeline);
    game->SetEventManager(&eventManager);
    eventManager.SetTimeline(&timeline);
    game->SetMode(NetworkMode::CLIENT);
    game->SetMemory(&allocator);

    // Set up replay manager
    replayManager.SetEntityManager(&entityManager);
    game->SetReplayManager(&replayManager);

    // Connect EventManager to ReplayManager for input recording
    eventManager.SetInputRecordingCallback([this](const EventData& data) {
        replayManager.RecordInput(data);
    });

    // Start worker threads
    physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
    renderThread = std::thread(&Application::RenderThreadFunction, this);
    networkThread = std::thread(&Application::NetworkThreadFunction, this);

    while(!rendererInitialized.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Initialize NetworkManager and connect to server
    networkManager.SetEntityManager(&entityManager);
    if (!networkManager.Connect(serverAddress)) {
        std::cout << "Failed to connect to server at " << serverAddress << "\n";
        return;
    }

    // Set network manager reference for game
    game->SetNetworkManager(&networkManager);

    // Run game start method
    //game->OnStart();

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

void ServerSignalHandler(int signal) {
    if (signal == SIGINT && g_running) {
        g_running->store(false);
    }
}

}
