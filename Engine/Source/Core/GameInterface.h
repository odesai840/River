#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "Physics/physics.h"

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
    void setPhysicsRef(Physics* physics) {this->physicsRef = physics;}
    Physics * getPhysicsRef() const {return physicsRef;}
    
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
    // Flip an entity's sprite
    void FlipSprite(uint32_t entityID, bool flipX, bool flipY);
    // Checks if a key is pressed
    bool IsKeyPressed(SDL_Scancode key);
    //Set gravity
    void setPhysics(const float gravity);
    //Get gravity
    float getPhysics();

    
private:
    // Internal renderer reference (internal use only)
    Renderer* rendererRef = nullptr;
    // Internal input reference (internal use only)
    Input* inputRef = nullptr;
    // Internal physics reference (internal use only)
    Physics* physicsRef = nullptr;
};

}

#endif
