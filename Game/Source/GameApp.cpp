#include "Application.h"
#include "MainBehavior.h"

int main(int argc, char* argv[]) {
    // River application
    RiverCore::Application app;
    // Master script for gameplay logic
    MainBehavior mainBehavior;
    // Start the core game loop
    app.Run(&mainBehavior);
    return 0;
}
