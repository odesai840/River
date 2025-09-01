#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL_scancode.h>

namespace RiverCore {

class Input {
public:
    Input();
    // Returns if a key was pressed
    bool IsKeyPressed(SDL_Scancode scancode);
};

}

#endif
