#include "GameInterface.h"
#include <iostream>
namespace RiverCore {

uint32_t GameInterface::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation,
    float Xscale, float Yscale, bool physEnabled)
{
    if(entityManagerRef) {
        return entityManagerRef->AddEntity(spritePath, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

uint32_t GameInterface::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps,
    float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled)
{
    if(entityManagerRef) {
        return entityManagerRef->AddAnimatedEntity(spritePath, totalFrames, fps, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

void GameInterface::RemoveEntity(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->RemoveEntity(entityID);
    }
}

void GameInterface::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    if(entityManagerRef) {
        entityManagerRef->UpdateEntityPosition(entityID, newX, newY);
    }
}

std::vector<std::pair<uint32_t, int>> GameInterface::GetEntityCollisions(uint32_t entityID) {
    if(entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            return entity->collider.GetCollisions();
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
    if(entityManagerRef) {
        entityManagerRef->FlipSprite(entityID, flipX, flipY);
    }
}

bool GameInterface::GetFlipX(uint32_t entityID) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipX(entityID);
    }
    return false;
}

bool GameInterface::GetFlipY(uint32_t entityID) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipY(entityID);
    }
    return false;
}

bool GameInterface::GetFlipState(uint32_t entityID, bool& flipX, bool& flipY) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipState(entityID, flipX, flipY);
    }
    flipX = false;
    flipY = false;
    return false;
}

void GameInterface::ToggleFlipX(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->ToggleFlipX(entityID);
    }
}

void GameInterface::ToggleFlipY(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->ToggleFlipY(entityID);
    }
}

void GameInterface::SetColliderType(uint32_t entityID, ColliderType type) {
    if(entityManagerRef) {
        entityManagerRef->SetColliderType(entityID, type);
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
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->ApplyForce(*entity, Vec2(forceX, forceY));
        }
    }
}

void GameInterface::ApplyImpulse(uint32_t entityID, float impulseX, float impulseY) {
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->ApplyImpulse(*entity, Vec2(impulseX, impulseY));
        }
    }
}

void GameInterface::SetVelocity(uint32_t entityID, float velX, float velY) {
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->SetVelocity(*entity, Vec2(velX, velY));
        }
    }
}

Vec2 GameInterface::GetVelocity(uint32_t entityID) {
    if (entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            return entity->velocity;
        }
    }
    return Vec2::zero();
}

void GameInterface::SetPosition(uint32_t entityID, float newX, float newY) {
    if (entityManagerRef) {
        entityManagerRef->SetPosition(entityID, Vec2(newX, newY));
    }
}

Vec2 GameInterface::GetPosition(uint32_t entityID) {
    if (entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            return entity->position;
        }
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

void GameInterface::SetTimeScale(float scale) {
    if (timelineRef) {
        timelineRef->SetTimeScale(scale);
    }
}

float GameInterface::GetTimeScale() const {
    if (timelineRef) {
        return timelineRef->GetTimeScale();
    }
    return 1.0f;
}

void GameInterface::IncreaseTimeScale() {
    if (timelineRef) {
        timelineRef->IncreaseTimeScale();
    }
}

void GameInterface::DecreaseTimeScale() {
    if (timelineRef) {
        timelineRef->DecreaseTimeScale();
    }
}

void GameInterface::SetPaused(bool isPaused) {
    if (timelineRef) {
        timelineRef->SetPaused(isPaused);
    }
}

bool GameInterface::IsPaused() const {
    if (timelineRef) {
        return timelineRef->IsPaused();
    }
    return false;
}

void GameInterface::sendVel(uint32_t playerId, float velx, float vely) {
    if (!clientRef) {
        std::cout << "bad";
    }
    if (clientRef) {
        clientRef -> sendVel(playerId, velx, vely);
    }
}

bool GameInterface::update(uint32_t& playerId, float& velx, float& vely) {
    if (clientRef) {
        clientRef->update(playerId, velx, vely);
        return true;
    };
    return false;
}

}