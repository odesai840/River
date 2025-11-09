#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

#include "Renderer/Renderer.h"
#include "Renderer/EntityManager.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Timeline.h"
#include "Networking/NetworkManager.h"
#include "NetworkMode.h"
#include "EventHandler/EventManager.h"

namespace RiverCore {

// Forward declarations
class Server;
class ServerInputManager;

class GameInterface {
public:
    virtual ~GameInterface() = default;

    // Runs before the main game loop at the start of the game
    virtual void OnStart() {}
    // Runs for every frame
    virtual void OnUpdate(float deltaTime) {}

    // Optional callbacks for server mode
    virtual void OnClientConnected(uint32_t clientID) {}
    virtual void OnClientDisconnected(uint32_t clientID) {}

    // Set the internal renderer reference (for use in the engine core only)
    void SetRenderer(Renderer* renderer) { this->rendererRef = renderer; }
    // Set the internal input system reference (for use in the engine core only)
    void SetInput(Input* input) { this->inputRef = input; }
    // Set the internal physics system reference (for use in the engine core only)
    void SetPhysicsRef(Physics* physics) {this->physicsRef = physics;}
    // Set the internal entity manager reference (for use in the engine core only)
    void SetEntityManager(EntityManager* entityManager) { this->entityManagerRef = entityManager; }
    // Set timeline reference (for use in engine core only)
    void SetTimeline(Timeline* timeline) { this->timelineRef = timeline; }
    // Set network manager reference (for use in engine core only)
    void SetNetworkManager(NetworkManager* networkManager) { this->networkManagerRef = networkManager; }
    // Set server input manager reference (for server mode only)
    void SetInputManager(ServerInputManager* inputManager) { this->serverInputManagerRef = inputManager; }
    // Set server reference (for server mode only)
    void SetServerRef(Server* server) { this->serverRef = server; }
    // Set event manager reference (for use in engine core only)
    void SetEventManager(EventManager* eventManager) { this->eventManagerRef = eventManager; }
    // Set network mode (for use in engine core only)
    void SetMode(NetworkMode mode) { this->currentMode = mode; }
    // Set headless server flag (for server mode only)
    void SetHeadlessServer(bool headless) { headlessServer = headless; }
    
protected:
    // Add an entity to the scene
    uint32_t AddEntity(const char* spritePath, float Xpos = 0.0f, float Ypos = 0.0f, float rotation = 0.0f,
        float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Add an animated entity to the scene
    uint32_t AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos = 0.0f, float Ypos = 0.0f,
        float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
    // Removes an entity from the screen
    void RemoveEntity(uint32_t entityID);
    // Update an entity's position given its ID
    void UpdateEntityPosition(uint32_t entityID, float newX, float newY);
    // Returns an entity's collisions given its ID
    std::vector<std::pair<uint32_t, int>> GetEntityCollisions(uint32_t entityID);
    // Flip an entity's sprite
    void FlipSprite(uint32_t entityID, bool flipX, bool flipY);
    // Returns the X-axis flip state of an entity's sprite
    bool GetFlipX(uint32_t entityID);
    // Get the Y-axis flip state of an entity's sprite
    bool GetFlipY(uint32_t entityID);
    // Get the flip state of an entity's sprite
    bool GetFlipState(uint32_t entityID, bool& flipX, bool& flipY);
    // Toggles the X-axis flip state of an entity's sprite
    void ToggleFlipX(uint32_t entityID);
    // Toggles the Y-axis flip state of an entity's sprite
    void ToggleFlipY(uint32_t entityID);
    // Sets an entity's collider type
    void SetColliderType(uint32_t entityID, ColliderType type);
    // Checks if a key is pressed
    bool IsKeyPressed(SDL_Scancode key);
    // Set gravity
    void SetGravity(float gravity);
    // Get Gravity
    float GetGravity();
    // Applies a force to an entity
    void ApplyForce(uint32_t entityID, float forceX, float forceY);
    // Applies an impulse to an entity
    void ApplyImpulse(uint32_t entityID, float impulseX, float impulseY);
    // Sets an entity's velocity
    void SetVelocity(uint32_t entityID, float velX, float velY);
    // Gets an entity's velocity
    Vec2 GetVelocity(uint32_t entityID);
    // Sets an entity's position
    void SetPosition(uint32_t entityID, float newX, float newY);
    // Gets an entity's position
    Vec2 GetPosition(uint32_t entityID);
    // Toggles scaling mode
    void ToggleScalingMode();
    // Toggles collision debug boxes
    void ToggleDebugCollisions();

    // Sets camera position
    void SetCameraPosition(const Vec2& pos);
    // Sets camera position
    void SetCameraPosition(float x, float y);
    // Gets camera position
    Vec2 GetCameraPosition() const;
    // Moves camera
    void MoveCamera(const Vec2& delta);
    // Moves camera
    void MoveCamera(float deltaX, float deltaY);
    // Moves camera to a new position instantly
    void SnapCameraToPosition(const Vec2& pos);
    // Sets camera zoom
    void SetCameraZoom(float zoom);
    // Gets camera zoom
    float GetCameraZoom() const;
    // Sets camera zoom limits
    void SetCameraZoomLimits(float min, float max);
    // Sets camera bounds
    void SetCameraBounds(const Vec2& min, const Vec2& max);
    // Sets camera bounds
    void SetCameraBounds(float minX, float minY, float maxX, float maxY);
    // Enables/disables camera bounds
    void EnableCameraBounds(bool enable);
    // Sets camera to follow a target position with smoothing
    void FollowCameraTarget(const Vec2& targetPos, float smoothing, float deltaTime);
    // Sets camera to follow a target position with smoothing
    void FollowCameraTarget(float targetX, float targetY, float smoothing, float deltaTime);
    // Sets camera follow dead zone
    void SetCameraDeadZone(float width, float height);
    // Checks if the camera is in the dead zone
    bool IsCameraInDeadZone(const Vec2& targetPos) const;
    // Camera viewport query (what world area is visible on screen)
    Vec2 GetVisibleWorldMin() const;
    // Camera viewport query (what world area is visible on screen)
    Vec2 GetVisibleWorldMax() const;
    // Camera viewport query (what world area is visible on screen)
    bool IsWorldPositionVisible(const Vec2& worldPos, const Vec2& size) const;
    
    // Sets the internal engine time scale to a custom time scale
    void SetTimeScale(float scale);
    // Gets the internal engine time scale
    float GetTimeScale() const;
    // Increases the internal engine time scale to the next default setting (eg. 1.0x -> 2.0x)
    void IncreaseTimeScale();
    // Decreases the internal engine time scale to the next default setting (eg. 1.0x -> 0.5x)
    void DecreaseTimeScale();
    // Sets the internal engine pause state
    void SetPaused(bool isPaused);
    // Returns the current engine pause state
    bool IsPaused() const;

    // Mode detection
    bool IsServer() const { return currentMode == NetworkMode::SERVER; }
    bool IsClient() const { return currentMode == NetworkMode::CLIENT; }
    bool IsStandalone() const { return currentMode == NetworkMode::STANDALONE; }
    bool IsHeadlessServer() const { return headlessServer; }

    // Gets the input state of a connected client
    InputState GetInputForClient(uint32_t clientID);
    // Gets the IDs of all connected clients
    std::vector<uint32_t> GetConnectedClients();
    // Gets the entity id of a connected client
    uint32_t GetPlayerEntityForClient(uint32_t clientID);
    // Sets the entity id of a connected client (useful for listen-server host player)
    void RegisterPlayerEntity(uint32_t clientID, uint32_t entityID);
    // Broadcasts entity spawns to connected clients
    void BroadcastEntitySpawn(uint32_t entityID, uint32_t ownerClientID = 0, uint32_t excludeClientID = 0);
    // Broadcasts entity despawns to connected clients
    void BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID = 0);

    // Sends client input states to the server
    void SendInputToServer(const std::unordered_map<std::string, bool>& buttons);
    // Gets the local player's client ID
    uint32_t GetLocalClientId();
    // Gets the local player's entity ID
    uint32_t GetLocalPlayerEntity();

    // Registers an Event
    void Register(int type, Event e);
    // Deregisters event
    void Deregister(int type);
    // Queues event
    void Queue(int type, EventData data = EventData());
    // Raises events in order of queue
    void Raise();

private:
    // Internal renderer reference (internal use only)
    Renderer* rendererRef = nullptr;
    // Internal input reference (internal use only)
    Input* inputRef = nullptr;
    // Internal physics reference (internal use only)
    Physics* physicsRef = nullptr;
    // Internal entity manager reference (internal use only)
    EntityManager* entityManagerRef = nullptr;
    // Internal timeline reference (internal use only)
    Timeline* timelineRef = nullptr;
    // Internal network manager reference (internal use only)
    NetworkManager* networkManagerRef = nullptr;
    // Internal server input manager reference (internal use and server mode only)
    ServerInputManager* serverInputManagerRef = nullptr;
    // Internal server reference (internal use and server mode only)
    Server* serverRef = nullptr;
    // Internal event manager refference (internal use only)
    EventManager* eventManagerRef = nullptr;
    
    // Current network mode
    NetworkMode currentMode = NetworkMode::STANDALONE;
    // Headless server flag (server mode only)
    bool headlessServer = false;
};

}

#endif
