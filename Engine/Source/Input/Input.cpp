#include "Input.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>

namespace RiverCore {

    Input::Input() {
        
    }

    bool Input::IsKeyPressed(SDL_Scancode scancode) {
        // Get the current state of the keyboard
        int numKeys = 0;
        const bool* keybordState = SDL_GetKeyboardState(&numKeys);

        //Check that the scancode is between 0 and the highest-value keyboard key
        if (0 < scancode && scancode < numKeys) {
            // If the scancode is valid, return whether that key is pressed
            return keybordState[scancode];
        }

        // If the scancode is invalid, return false
        return false;
    }

}
