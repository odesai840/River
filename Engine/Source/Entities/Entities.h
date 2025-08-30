#ifndef ENTITIES.H
#define ENTITIES.H
#include "Renderer/Renderer.h"
#include "Core/Window.h"


namespace RiverCore
{
    class Entities
    {
    public:
        Entities();
        void LoadSpriteSheet(char *fileName);
        void LoadPositionAndFrame(int currentFrame, int spriteWidth, int spriteHeight, int x_pos, int y_pos);
    };
    
}

#endif