#include "Application.h"
#include "MainBehavior.h"
#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
    // River application
    RiverCore::Application app;
    // Master script for gameplay logic
    MainBehavior mainBehavior;

    // Parse command line arguments
    if (argc > 1) {
        std::string arg1 = argv[1];

        if (arg1 == "--server") {
            // Run as dedicated server
            std::cout << "Starting River server...\n";
            app.RunServer();
            return 0;
        }
        else if (arg1 == "--client") {
            // Run as client
            std::string serverAddress = "localhost";
            if (argc > 2) {
                serverAddress = argv[2];
            }
            std::cout << "Starting River client, connecting to: " << serverAddress << "\n";
            app.RunClient(serverAddress, &mainBehavior);
            return 0;
        }
        else {
            std::cout << "Unknown argument: " << arg1 << "\n";
            std::cout << "Usage: River [--server | --client [address]]\n";
            return 1;
        }
    }

    // Default: Run standalone
    std::cout << "Starting River in standalone mode...\n";
    app.Run(&mainBehavior);
    return 0;
}
