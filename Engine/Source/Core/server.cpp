#include "server.h"
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
static zmq::socket_t* repSocket = nullptr;  // For connection handshake (port 5556)
static zmq::socket_t* pubSocket = nullptr;  // For broadcasting game state (port 5555)

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

        // Start two main processing threads
        std::thread connectionThread(&Server::ProcessConnectionRequests, this);
        std::thread broadcastThread(&Server::BroadcastGameState, this);

        std::cout << "Server started successfully. Connection handshake on port 5556, broadcasting on port 5555\n";

        // Wait for threads to complete
        connectionThread.join();
        broadcastThread.join();

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
        // Create REP socket for connection handshake
        repSocket = new zmq::socket_t(context, zmq::socket_type::rep);
        repSocket->bind("tcp://*:5556");

        // Create PUB socket for broadcasting game state
        pubSocket = new zmq::socket_t(context, zmq::socket_type::pub);
        pubSocket->bind("tcp://*:5555");

        // Set high water mark to prevent message queuing issues
        int hwm = 1000;
        pubSocket->set(zmq::sockopt::sndhwm, hwm);

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

    if (pubSocket) {
        try {
            pubSocket->close();
            delete pubSocket;
            pubSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing PUB socket: " << e.what() << "\n";
        }
    }
}

void Server::ProcessConnectionRequests() {
    std::cout << "Connection handler started - listening on port 5556\n";

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
                std::cout << "Received connection request: " << requestStr << "\n";

                std::istringstream iss(requestStr);
                std::string command;
                iss >> command;

                std::string response;
                if (command == "CONNECT") {
                    uint32_t newClientId = HandleConnect();
                    response = "CONNECTED " + std::to_string(newClientId) + " 0.0 0.0";
                } else if (command == "DISCONNECT") {
                    uint32_t clientId;
                    if (iss >> clientId) {
                        HandleDisconnect(clientId);
                        response = "DISCONNECTED";
                    } else {
                        response = "ERROR Invalid disconnect format";
                    }
                } else if (command == "POSITION") {
                    uint32_t clientId;
                    float x, y;
                    if (iss >> clientId >> x >> y) {
                        UpdateClientPosition(clientId, x, y);
                        response = "OK";
                    } else {
                        response = "ERROR Invalid position format";
                    }
                } else {
                    response = "ERROR Unknown command: " + command;
                }

                // Send response back to client
                zmq::message_t reply(response.size());
                memcpy(reply.data(), response.data(), response.size());
                repSocket->send(reply, zmq::send_flags::none);

                std::cout << "Sent connection response: " << response << "\n";
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

void Server::BroadcastGameState() {
    std::cout << "Broadcast handler started - broadcasting on port 5555 at 60Hz\n";

    while (running.load()) {
        try {
            auto startTime = std::chrono::steady_clock::now();

            // Broadcast position updates for clients whose positions have changed
            {
                std::lock_guard<std::mutex> lock(clientDataMutex);
                for (auto& [clientId, data] : clientData) {
                    if (data.positionChanged) {
                        BroadcastPositionUpdate(clientId, data.x, data.y);
                        data.positionChanged = false; // Reset the flag
                    }
                }
            }

            // Calculate sleep time to maintain 60Hz
            auto endTime = std::chrono::steady_clock::now();
            auto processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            auto sleepTime = std::chrono::milliseconds(BROADCAST_INTERVAL_MS) - processingTime;

            if (sleepTime > std::chrono::milliseconds(0)) {
                std::this_thread::sleep_for(sleepTime);
            }

        } catch (const std::exception& e) {
            std::cout << "Error in broadcast handler: " << e.what() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(BROADCAST_INTERVAL_MS));
        }
    }

    std::cout << "Broadcast handler stopped\n";
}

uint32_t Server::HandleConnect() {
    uint32_t newClientId = nextClientID.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(clientDataMutex);
        clientData[newClientId] = ClientData(0.0f, 0.0f); // Spawn at origin
    }

    std::cout << "Client " << newClientId << " connected. Total clients: " << clientData.size() << "\n";

    // Broadcast to all other clients that a new client connected
    BroadcastClientConnected(newClientId, 0.0f, 0.0f);

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

    // Broadcast to all remaining clients that this client disconnected
    BroadcastClientDisconnected(clientId);
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
            it->second.positionChanged = true;
        }
    }
}

void Server::RemoveClient(uint32_t clientId) {
    HandleDisconnect(clientId);
}

void Server::BroadcastClientConnected(uint32_t clientId, float x, float y) {
    std::ostringstream oss;
    oss << "CLIENT_CONNECTED:" << clientId << " " << x << " " << y;
    BroadcastMessage(oss.str());
}

void Server::BroadcastClientDisconnected(uint32_t clientId) {
    std::ostringstream oss;
    oss << "CLIENT_DISCONNECTED:" << clientId;
    BroadcastMessage(oss.str());
}

void Server::BroadcastPositionUpdate(uint32_t clientId, float x, float y) {
    std::ostringstream oss;
    oss << "POSITION_UPDATE:" << clientId << " " << x << " " << y << " "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    BroadcastMessage(oss.str());
}

void Server::BroadcastMessage(const std::string& message) {
    if (!pubSocket || !running.load()) {
        return;
    }

    try {
        zmq::message_t zmqMessage(message.size());
        memcpy(zmqMessage.data(), message.data(), message.size());
        pubSocket->send(zmqMessage, zmq::send_flags::dontwait);
    } catch (const zmq::error_t& e) {
        if (e.num() != ETERM) {
            std::cout << "Failed to broadcast message: " << e.what() << "\n";
        }
    }
}

}