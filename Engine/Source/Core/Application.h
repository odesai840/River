#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Renderer/Renderer.h"

namespace RiverCore {

class Application {
public:
    Application();
    ~Application();

    // Initializes engine resources
    void Init();
    // Starts the core application loop
    void Run();

private:
    // Internal window class
    Window window;
    // Internal renderer class
    Renderer renderer;
};

}

#endif
