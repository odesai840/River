#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "GameInterface.h"
#include "NetworkMode.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Renderer/EntityManager.h"
#include "Timeline.h"
#include "Networking/NetworkManager.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>

namespace RiverCore {

// Forward declarations
class Server;

class Application {
public:
    Application();
    ~Application();

    // Initializes engine resources
    void Init();
    // Starts the core application loop (standalone mode)
    void Run(GameInterface* game);
    // Starts the server loop
    void RunServer(GameInterface* game, bool headless = true);
    // Starts the client loop with server connection
    void RunClient(const std::string& serverAddress, GameInterface* game);

    // Provides access to the entity manager
    EntityManager& GetEntityManager() { return entityManager; }

private:
    // Internal window class
    Window window;
    // Internal renderer class
    Renderer renderer;
    // Internal input system class
    Input input;
    // Internal physics class
    Physics physics;
    // Internal entity manager class
    EntityManager entityManager;
    // Timeline for time scaling and pause management
    Timeline timeline;
    // Network manager for client networking
    NetworkManager networkManager;

    // Current network mode
    NetworkMode currentMode = NetworkMode::STANDALONE;

    // Game interface reference for thread access
    GameInterface* gameRef = nullptr;

    // Atomic boolean to control thread loops
    std::atomic<bool> running{true};
    // Physics thread reference
    std::thread physicsThread;
    // Render thread reference
    std::thread renderThread;
    // Network thread reference
    std::thread networkThread;

    // Mutex for renderer synchronization
    std::mutex renderMutex;
    // Condition variable for renderer synchronization
    std::condition_variable renderCondition;
    // Atomic boolean to control the render thread loop
    std::atomic<bool> renderReady{false};

    // Physics thread function
    void PhysicsThreadFunction();
    // Render thread function
    void RenderThreadFunction();
    // Listen-server render thread function (uses server's EntityManager)
    void RenderThreadFunction_ListenServer(Server* server);
    // Network thread function
    void NetworkThreadFunction();

    // Fixed 60 Hz timestep for physics updates
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
    // Maximum frame time for rendering
    static constexpr float MAX_FRAME_TIME = 0.25f;
};

}

#endif
