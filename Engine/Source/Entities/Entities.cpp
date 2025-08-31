#include "Entities.h"
#include <SDL3_image/SDL_image.h>

namespace RiverCore
{
    /* Struct to store entity and it's position
    Code from Module 2 Boiler Plate Activity */
    struct Entity
    {
        SDL_Texture *texture = nullptr;
    };
    //Create an object for the singular entity
    Entity entity;
    
    
    
    Entities::Entities()
    {
        
    }

    //Intializes the renderer
    void Entities::Init(){
        renderer.Init(window.GetNativeWindow());
    }
    //Creates the renderer object
    SDL_Renderer * renderObject = renderer.GetRenderer();

    //Loads the whole spritesheet into the entity
    void Entities::LoadSpriteSheet(char *fileName)
    {
        SDL_Surface *spriteSheet = IMG_Load(fileName);
        entity.texture=SDL_CreateTextureFromSurface(renderObject, spriteSheet);
        SDL_DestroySurface(spriteSheet);
    }

    //Loads the frame of the entity and the position to where to draw the entity, and draws the entity
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
