#include "GameInterface.h"

namespace RiverCore {

uint32_t GameInterface::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled) {
    if(rendererRef) {
        return rendererRef->AddEntity(spritePath, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

uint32_t GameInterface::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos, float Ypos,
    float rotation, float Xscale, float Yscale, bool physEnabled)
{
    if(rendererRef) {
        return rendererRef->AddAnimatedEntity(spritePath, totalFrames, fps, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

void GameInterface::RemoveEntity(uint32_t entityID) {
    if(rendererRef) {
        rendererRef->RemoveEntity(entityID);
    }
}

void GameInterface::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    if(rendererRef) {
        rendererRef->UpdateEntityPosition(entityID, newX, newY);
    }
}

std::vector<std::pair<uint32_t, int>> GameInterface::GetEntityCollisions(uint32_t entityID) {
    if(rendererRef) {
        Entity* entity = rendererRef->GetEntityByID(entityID);
        if (entity) {
            return entity->collisions;
        }
    }
    return std::vector<std::pair<uint32_t, int>>();
}

bool GameInterface::IsKeyPressed(SDL_Scancode key) {
    if(inputRef) {
        return inputRef->IsKeyPressed(key);
    }
    return false;
}

void GameInterface::FlipSprite(uint32_t entityID, bool flipX, bool flipY) {
    if(rendererRef) {
        rendererRef->FlipSprite(entityID, flipX, flipY);
    }
}

void GameInterface::SetGravity(float gravity) {
    if (physicsRef){
        physicsRef->SetGravity(gravity);
    }
}

float GameInterface::GetGravity(){
    if (physicsRef){
        return physicsRef->GetGravity();
    }
    return 0.0f;
}

void GameInterface::ApplyForce(uint32_t entityID, float forceX, float forceY) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        physicsRef->ApplyForce(*entity, Vec2(forceX, forceY));
    }
}

void GameInterface::ApplyImpulse(uint32_t entityID, float impulseX, float impulseY) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        physicsRef->ApplyImpulse(*entity, Vec2(impulseX, impulseY));
    }
}

void GameInterface::SetVelocity(uint32_t entityID, float velX, float velY) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        physicsRef->SetVelocity(*entity, Vec2(velX, velY));
    }
}

Vec2 GameInterface::GetVelocity(uint32_t entityID) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        return entity->velocity;
    }
    return Vec2::zero();
}

void GameInterface::SetPosition(uint32_t entityID, float newX, float newY) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        rendererRef->SetPosition(*entity, Vec2(newX, newY));
    }
}

Vec2 GameInterface::GetPosition(uint32_t entityID) {
    Entity* entity = rendererRef->GetEntityByID(entityID);
    if (entity) {
        return entity->position;
    }
    return Vec2::zero();
}

void GameInterface::ToggleScalingMode() {
    if (rendererRef) {
        rendererRef->ToggleScalingMode();
    }
}

void GameInterface::ToggleDebugCollisions() {
    if (rendererRef) {
        rendererRef->ToggleDebugCollisions();
    }
}

}