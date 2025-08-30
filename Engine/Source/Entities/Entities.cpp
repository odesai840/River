#include "Entities.h"
#include <SDL_image.h>

namespace RiverCore
{
    /* Struct to store entity and it's position
    Code from Module 2 Boiler Plate Activity */
    struct Entity
    {
        SDL_Texture *texture = nullptr;
    };

    
    
    Entity entity;
    Renderer renderer;
    Window window;
    renderer.Init(window.GetNativeWindow());
    SDL_Renderer * renderObject = renderer.GetRenderer();

    Entities::Entities()
    {
        
    }

    void Entities::LoadSpriteSheet(char *fileName)
    {
        SDL_Surface *spriteSheet = IMG_Load(fileName);
        entity.texture=SDL_CreateTextureFromSurface(renderObject, spriteSheet);
        SDL_DestroySurface(spriteSheet);
    }

    void Entities::LoadPositionAndFrame(int currentFrame, int spriteWidth, int spriteHeight, int x_pos, int y_pos){
        SDL_FRect srcRect = {
            (float)(currentFrame * spriteWidth),
            0.0f,
            (float) spriteWidth,
            (float) spriteHeight,
        };

        SDL_FRect dstRect = {
            x_pos,
            y_pos,
            spriteWidth,
            spriteHeight,
        };

        SDL_RenderTexture(renderObject, entity.texture, &srcRect, &dstRect);
        SDL_RenderPresent(renderObject);
    }
}
