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

}