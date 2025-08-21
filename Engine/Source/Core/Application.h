#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"

namespace RiverCore {

class Application {
public:
    Application();
    virtual ~Application();
    void Init();
    void Run();

private:
    Window window;
    SDL_Renderer* renderer = nullptr;
};

}

#endif
