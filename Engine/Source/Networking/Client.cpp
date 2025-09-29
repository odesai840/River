#include "Client.h"
#include <zmq/zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace RiverCore {

// Global ZMQ context and sockets
static zmq::context_t context(1);
static zmq::socket_t* reqSocket = nullptr;

Client::Client() {
    lastPositionUpdate = std::chrono::steady_clock::now();
}

Client::~Client() {
    // Ensure proper cleanup
    Disconnect();
}

bool Client::Connect(const std::string& serverAddress) {
    if (connected.load()) {
        std::cout << "Client is already connected\n";
        return true;
    }

    std::cout << "Connecting to server at " << serverAddress << "\n";

    try {
        InitializeSockets(serverAddress);

        // Perform connection handshake via REQ socket
        std::string connectMsg = "CONNECT";
        zmq::message_t request(connectMsg.size());
        memcpy(request.data(), connectMsg.data(), connectMsg.size());

        if (!reqSocket->send(request, zmq::send_flags::none)) {
            std::cout << "Failed to send CONNECT request\n";
            CleanupSockets();
            return false;
        }

        // Wait for response
        zmq::message_t reply;
        auto result = reqSocket->recv(reply, zmq::recv_flags::none);

        if (result) {
            std::string response(static_cast<char*>(reply.data()), reply.size());
            std::cout << "Server handshake response: " << response << "\n";

            std::istringstream iss(response);
            std::string status;
            iss >> status;

            if (status == "CONNECTED") {
                uint32_t assignedId;
                float startX, startY;
                if (iss >> assignedId >> startX >> startY) {
                    clientId = assignedId;
                    currentX = startX;
                    currentY = startY;
                    connected = true;

                    std::cout << "Connected successfully! Client ID: " << assignedId
                              << ", Starting position: (" << startX << ", " << startY << ")\n";

                    return true;
                } else {
                    std::cout << "Invalid CONNECTED response format\n";
                }
            } else {
                std::cout << "Connection failed: " << response << "\n";
            }
        } else {
            std::cout << "No response from server during handshake\n";
        }

    } catch (const zmq::error_t& e) {
        std::cout << "ZMQ error during connection: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error during connection: " << e.what() << "\n";
    }

    CleanupSockets();
    return false;
}

void Client::Disconnect() {
    if (!connected.load()) {
        return;
    }

    std::cout << "Disconnecting from server...\n";

    // Signal update loop to stop
    disconnecting = true;

    // Wait briefly for update loop to finish current operations
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Send disconnect message to server
    if (reqSocket && clientId.load() != 0) {
        std::lock_guard<std::mutex> lock(socketMutex);

        try {
            std::string disconnectMsg = "DISCONNECT " + std::to_string(clientId.load());
            zmq::message_t request(disconnectMsg.size());
            memcpy(request.data(), disconnectMsg.data(), disconnectMsg.size());

            // Send disconnect with reliable synchronous operation
            bool disconnectSent = false;
            for (int attempt = 0; attempt < 3 && !disconnectSent; ++attempt) {
                try {
                    if (reqSocket->send(request, zmq::send_flags::none)) {
                        // Wait for acknowledgment
                        zmq::message_t reply;
                        auto result = reqSocket->recv(reply, zmq::recv_flags::none);

                        if (result) {
                            std::string response(static_cast<char*>(reply.data()), reply.size());
                            if (response == "DISCONNECTED") {
                                std::cout << "Successfully sent disconnect message to server\n";
                                disconnectSent = true;
                            }
                        }
                    }
                } catch (const zmq::error_t& e) {
                    if (e.num() == EAGAIN && attempt < 2) {
                        std::cout << "Disconnect attempt " << (attempt + 1) << " timed out, retrying...\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    } else {
                        std::cout << "ZMQ error sending disconnect message: " << e.what() << "\n";
                        break;
                    }
                }
            }

            if (!disconnectSent) {
                std::cout << "Failed to send disconnect message after 3 attempts\n";
            }

        } catch (const std::exception& e) {
            std::cout << "Error sending disconnect message: " << e.what() << "\n";
        }
    }

    // Reset state
    connected = false;
    clientId = 0;
    disconnecting = false;

    {
        std::lock_guard<std::mutex> lock(otherClientsMutex);
        otherClients.clear();
    }

    CleanupSockets();
    std::cout << "Disconnected from server\n";
}

void Client::Update() {
    if (!connected.load() || disconnecting.load()) {
        return;
    }

    try {
        // Send position updates and get game state at 60Hz
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPositionUpdate);

        if (timeSinceLastUpdate.count() >= POSITION_UPDATE_INTERVAL_MS) {
            // Check again if we're disconnecting before socket operation
            if (!disconnecting.load()) {
                UpdateAndGetGameState();
                lastPositionUpdate = now;
            }
        }

    } catch (const std::exception& e) {
        std::cout << "Error in client update: " << e.what() << "\n";
    }
}

void Client::SendPosition(float x, float y) {
    if (!connected.load()) {
        return;
    }

    // Only mark as dirty if position actually changed
    if (currentX.load() != x || currentY.load() != y) {
        currentX = x;
        currentY = y;
        positionDirty = true;
    }
}

std::unordered_map<uint32_t, OtherClientData> Client::GetOtherClients() const {
    std::lock_guard<std::mutex> lock(otherClientsMutex);
    return otherClients; // Return copy for thread safety
}

void Client::InitializeSockets(const std::string& serverAddress) {
    try {
        // Create REQ socket for all client-server communication
        reqSocket = new zmq::socket_t(context, zmq::socket_type::req);
        std::string reqAddress = "tcp://" + serverAddress + ":5555";
        reqSocket->connect(reqAddress);

        // Set shorter timeout for 60Hz operation
        int timeout = 1000; // 1 second
        reqSocket->set(zmq::sockopt::rcvtimeo, timeout);
        reqSocket->set(zmq::sockopt::sndtimeo, timeout);

    } catch (const zmq::error_t& e) {
        CleanupSockets();
        throw std::runtime_error("Failed to initialize client sockets: " + std::string(e.what()));
    }
}

void Client::CleanupSockets() {
    if (reqSocket) {
        try {
            reqSocket->close();
            delete reqSocket;
            reqSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing REQ socket: " << e.what() << "\n";
        }
    }
}

void Client::UpdateAndGetGameState() {
    if (!reqSocket || !connected.load() || disconnecting.load()) {
        return;
    }

    std::lock_guard<std::mutex> lock(socketMutex);

    // Double-check after acquiring lock
    if (!reqSocket || !connected.load() || disconnecting.load()) {
        return;
    }

    try {
        // Create combined position update and game state request
        std::ostringstream oss;
        oss << "UPDATE_AND_GET_STATE " << clientId.load() << " " << currentX.load() << " " << currentY.load();
        std::string requestMsg = oss.str();

        // Send request
        zmq::message_t request(requestMsg.size());
        memcpy(request.data(), requestMsg.data(), requestMsg.size());

        if (reqSocket->send(request, zmq::send_flags::none)) {
            // Wait for response
            zmq::message_t reply;
            auto result = reqSocket->recv(reply, zmq::recv_flags::none);

            if (result) {
                std::string response(static_cast<char*>(reply.data()), reply.size());
                ParseGameStateResponse(response);
            }
        }

    } catch (const zmq::error_t& e) {
        if (e.num() != ETERM) {
            std::cout << "ZMQ error in update and get state: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error in update and get state: " << e.what() << "\n";
    }
}

void Client::ParseGameStateResponse(const std::string& response) {
    std::istringstream iss(response);
    std::string status;
    iss >> status;

    if (status != "OK") {
        std::cout << "Unexpected server response: " << response << "\n";
        return;
    }

    // Parse game state
    std::lock_guard<std::mutex> lock(otherClientsMutex);
    otherClients.clear();

    uint32_t clientId;
    float x, y;
    while (iss >> clientId >> x >> y) {
        // Don't track ourselves
        if (clientId != this->clientId.load()) {
            otherClients[clientId] = OtherClientData(x, y);
        }
    }
}


}