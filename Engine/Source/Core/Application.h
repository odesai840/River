#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "GameInterface.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "Physics/Physics.h"

namespace RiverCore {

class Application {
public:
    Application();
    ~Application();

    // Initializes engine resources
    void Init();
    // Starts the core application loop
    void Run(GameInterface* game);

private:
    // Internal window class
    Window window;
    // Internal renderer class
    Renderer renderer;
    // Internal input system class
    Input input;
    // Internal physics class
    Physics physics;
};

}

#endif
