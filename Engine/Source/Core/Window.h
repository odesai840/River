#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <string>

namespace RiverCore {

struct WindowProps {
    unsigned int Width;
    unsigned int Height;
    std::string Title;

    WindowProps(unsigned int width = 1280, unsigned int height = 720, std::string title = "River")
        : Width(width), Height(height), Title(title)
    {

    }
};

class Window {
public:
    Window(WindowProps props = WindowProps());
    ~Window();

    void SetNativeWindow(SDL_Window* window) { windowRef = window; }

    SDL_Window* GetNativeWindow() const { return windowRef; }
    unsigned int& GetWidth() { return winProps.Width; }
    unsigned int& GetHeight() { return winProps.Height; }
    std::string& GetTitle() { return winProps.Title; }

private:
    SDL_Window* windowRef = nullptr;
    WindowProps winProps;
};

}

#endif
