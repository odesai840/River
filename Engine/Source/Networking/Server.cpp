#include "Server.h"
#include <zmq/zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>

namespace RiverCore {

// Global ZMQ context and sockets
static zmq::context_t context(1);
static zmq::socket_t* repSocket = nullptr;

Server::Server() {
    
}

Server::~Server() {
    // Ensure proper cleanup
    Stop();
}

void Server::Start() {
    if (running.load()) {
        std::cout << "Server is already running\n";
        return;
    }

    std::cout << "Starting server...\n";

    try {
        InitializeSockets();
        running = true;

        // Start connection processing thread
        std::thread connectionThread(&Server::ProcessConnectionRequests, this);

        // Wait for thread to complete
        connectionThread.join();

    } catch (const std::exception& e) {
        std::cout << "Failed to start server: " << e.what() << "\n";
        CleanupSockets();
        running = false;
    }
}

void Server::Stop() {
    if (!running.load()) {
        return;
    }

    std::cout << "Stopping server...\n";
    running = false;

    // Give threads time to finish their current operations
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CleanupSockets();
    std::cout << "Server stopped successfully\n";
}

void Server::InitializeSockets() {
    try {
        // Create REP socket for all client-server communication
        repSocket = new zmq::socket_t(context, zmq::socket_type::rep);
        repSocket->bind("tcp://*:5555");

        // Set shorter timeout for responsive 60Hz operation
        int timeout = 100;
        repSocket->set(zmq::sockopt::rcvtimeo, timeout);

    } catch (const zmq::error_t& e) {
        CleanupSockets();
        throw std::runtime_error("Failed to initialize sockets: " + std::string(e.what()));
    }
}

void Server::CleanupSockets() {
    if (repSocket) {
        try {
            repSocket->close();
            delete repSocket;
            repSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing REP socket: " << e.what() << "\n";
        }
    }
}

void Server::ProcessConnectionRequests() {
    std::cout << "Server started successfully. Listening on port 5555\n";

    while (running.load()) {
        try {
            if (!repSocket) {
                std::cout << "REP socket is null, exiting connection handler\n";
                break;
            }

            // Non-blocking receive for connection requests
            zmq::message_t request;
            auto result = repSocket->recv(request, zmq::recv_flags::dontwait);

            if (result) {
                std::string requestStr(static_cast<char*>(request.data()), request.size());

                std::istringstream iss(requestStr);
                std::string command;
                iss >> command;

                std::string response;
                if (command == "CONNECT") {
                    std::cout << "Received connection request: " << requestStr << "\n";
                    uint32_t newClientId = HandleConnect();
                    response = "CONNECTED " + std::to_string(newClientId) + " 0.0 0.0";
                    std::cout << "Sent connection response: " << response << "\n";
                } else if (command == "DISCONNECT") {
                    std::cout << "Received disconnect request: " << requestStr << "\n";
                    uint32_t clientId;
                    if (iss >> clientId) {
                        HandleDisconnect(clientId);
                        response = "DISCONNECTED";
                    } else {
                        response = "ERROR Invalid disconnect format";
                    }
                    std::cout << "Sent disconnect response: " << response << "\n";
                } else if (command == "UPDATE_AND_GET_STATE") {
                    uint32_t clientId;
                    float x, y;
                    if (iss >> clientId >> x >> y) {
                        UpdateClientPosition(clientId, x, y);
                        response = GetGameStateResponse();
                    } else {
                        response = "ERROR Invalid update format";
                    }
                } else {
                    std::cout << "Received unknown request: " << requestStr << "\n";
                    response = "ERROR Unknown command: " + command;
                    std::cout << "Sent error response: " << response << "\n";
                }

                // Send response back to client
                zmq::message_t reply(response.size());
                memcpy(reply.data(), response.data(), response.size());
                repSocket->send(reply, zmq::send_flags::none);
            }

            // Small sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        } catch (const zmq::error_t& e) {
            if (e.num() != ETERM) {
                std::cout << "ZMQ error in connection handler: " << e.what() << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error in connection handler: " << e.what() << "\n";
        }
    }

    std::cout << "Connection handler stopped\n";
}


uint32_t Server::HandleConnect() {
    uint32_t newClientId = nextClientID.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(clientDataMutex);
        clientData[newClientId] = ClientData(0.0f, 0.0f);
    }

    std::cout << "Client " << newClientId << " connected. Total clients: " << clientData.size() << "\n";

    return newClientId;
}

void Server::HandleDisconnect(uint32_t clientId) {
    {
        std::lock_guard<std::mutex> lock(clientDataMutex);
        auto it = clientData.find(clientId);
        if (it != clientData.end()) {
            clientData.erase(it);
            std::cout << "Client " << clientId << " disconnected. Total clients: " << clientData.size() << "\n";
        }
    }
}

void Server::UpdateClientPosition(uint32_t clientId, float x, float y) {
    std::lock_guard<std::mutex> lock(clientDataMutex);
    auto it = clientData.find(clientId);
    if (it != clientData.end()) {
        // Only update if position actually changed
        if (it->second.x != x || it->second.y != y) {
            it->second.x = x;
            it->second.y = y;
            it->second.lastUpdate = std::chrono::steady_clock::now();
        }
    }
}

void Server::RemoveClient(uint32_t clientId) {
    HandleDisconnect(clientId);
}

std::string Server::GetGameStateResponse() {
    std::ostringstream oss;
    oss << "OK";

    std::lock_guard<std::mutex> lock(clientDataMutex);
    for (const auto& [clientId, data] : clientData) {
        oss << " " << clientId << " " << data.x << " " << data.y;
    }

    return oss.str();
}

}