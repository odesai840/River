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

void GameInterface::setGravity(float gravity) {
    if (physicsRef){
        physicsRef->setGravity(gravity);
    }
}

float GameInterface::getGravity(){
    if (physicsRef){
        return physicsRef->getGravity();
    }
    return 0.0f;
}

void GameInterface::ToggleScalingMode() {
    if (rendererRef) {
        rendererRef->ToggleScalingMode();
    }
}

}