#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>

namespace RiverCore {

class Input {
public:
    Input();
    bool IsKeyPressed(SDL_Scancode scancode);
};

}

#endif
