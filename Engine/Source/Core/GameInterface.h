#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

#include "Renderer/Renderer.h"
#include "Renderer/EntityManager.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "client.h"

namespace RiverCore {

class GameInterface {
public:
    virtual ~GameInterface() = default;

    // Runs before the main game loop at the start of the game
    virtual void OnStart() {}
    // Runs for every frame
    virtual void OnUpdate(float deltaTime) {}

    // Set the internal renderer reference (for use in the engine core only)
    void SetRenderer(Renderer* renderer) { this->rendererRef = renderer; }
    // Set the internal input system reference (for use in the engine core only)
    void SetInput(Input* input) { this->inputRef = input; }
    // Set the internal physics system reference (for use in the engine core only)
    void SetPhysicsRef(Physics* physics) {this->physicsRef = physics;}
    // Set the internal entity manager reference (for use in the engine core only)
    void SetEntityManager(EntityManager* entityManager) { this->entityManagerRef = entityManager; }
    
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
    // Send velocity update
    void sendVel(uint32_t playerId, float velx, float vely);
    //Handle updates
    bool update(uint32_t& playerId, float& velx, float& vely);
    
private:
    // Internal renderer reference (internal use only)
    Renderer* rendererRef = nullptr;
    // Internal input reference (internal use only)
    Input* inputRef = nullptr;
    // Internal physics reference (internal use only)
    Physics* physicsRef = nullptr;
    // Internal entity manager reference (internal use only)
    EntityManager* entityManagerRef = nullptr;
    // Internal Server manager refernce (internal use only)
    Client* clientRef = nullptr;
};

}

#endif
