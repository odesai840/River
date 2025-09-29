#include "client.h"
#include <zmq/zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace RiverCore {

// Global ZMQ context and sockets
static zmq::context_t context(1);
static zmq::socket_t* reqSocket = nullptr;  // For connection handshake (port 5556)
static zmq::socket_t* subSocket = nullptr;  // For receiving broadcasts (port 5555)

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

        // Step 1: Perform connection handshake via REQ socket
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

                    // Step 2: Subscribe to broadcast messages
                    subSocket->set(zmq::sockopt::subscribe, ""); // Subscribe to all messages

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

    // Send disconnect message to server
    if (reqSocket && clientId.load() != 0) {
        try {
            std::string disconnectMsg = "DISCONNECT " + std::to_string(clientId.load());
            zmq::message_t request(disconnectMsg.size());
            memcpy(request.data(), disconnectMsg.data(), disconnectMsg.size());

            // Send disconnect with short timeout
            reqSocket->send(request, zmq::send_flags::dontwait);

            // Try to receive acknowledgment but don't wait long
            zmq::message_t reply;
            reqSocket->recv(reply, zmq::recv_flags::dontwait);

        } catch (const std::exception& e) {
            std::cout << "Error sending disconnect message: " << e.what() << "\n";
        }
    }

    // Reset state
    connected = false;
    clientId = 0;

    {
        std::lock_guard<std::mutex> lock(otherClientsMutex);
        otherClients.clear();
    }

    CleanupSockets();
    std::cout << "Disconnected from server\n";
}

void Client::Update() {
    if (!connected.load()) {
        return;
    }

    try {
        // Process incoming broadcast messages
        ProcessIncomingMessages();

        // Send position updates if needed
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPositionUpdate);

        if (positionDirty.load() && timeSinceLastUpdate.count() >= POSITION_UPDATE_INTERVAL_MS) {
            SendPositionUpdateToServer();
            lastPositionUpdate = now;
            positionDirty = false;
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
        // Create REQ socket for connection handshake
        reqSocket = new zmq::socket_t(context, zmq::socket_type::req);
        std::string reqAddress = "tcp://" + serverAddress + ":5556";
        reqSocket->connect(reqAddress);

        // Set connection timeout
        int timeout = 5000; // 5 seconds
        reqSocket->set(zmq::sockopt::rcvtimeo, timeout);
        reqSocket->set(zmq::sockopt::sndtimeo, timeout);

        // Create SUB socket for receiving broadcasts
        subSocket = new zmq::socket_t(context, zmq::socket_type::sub);
        std::string subAddress = "tcp://" + serverAddress + ":5555";
        subSocket->connect(subAddress);

        // Set high water mark to prevent message queuing issues
        int hwm = 1000;
        subSocket->set(zmq::sockopt::rcvhwm, hwm);

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

    if (subSocket) {
        try {
            subSocket->close();
            delete subSocket;
            subSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing SUB socket: " << e.what() << "\n";
        }
    }
}

void Client::ProcessIncomingMessages() {
    if (!subSocket || !connected.load()) {
        return;
    }

    // Process all available messages (non-blocking)
    while (true) {
        try {
            zmq::message_t message;
            auto result = subSocket->recv(message, zmq::recv_flags::dontwait);

            if (!result) {
                break; // No more messages available
            }

            std::string messageStr(static_cast<char*>(message.data()), message.size());
            ParseBroadcastMessage(messageStr);

        } catch (const zmq::error_t& e) {
            if (e.num() == EAGAIN) {
                break; // No more messages available
            } else if (e.num() != ETERM) {
                std::cout << "ZMQ error receiving broadcast: " << e.what() << "\n";
                break;
            }
        } catch (const std::exception& e) {
            std::cout << "Error processing broadcast message: " << e.what() << "\n";
            break;
        }
    }
}

void Client::ParseBroadcastMessage(const std::string& message) {
    // Parse message
    size_t colonPos = message.find(':');
    if (colonPos == std::string::npos) {
        std::cout << "Invalid broadcast message format: " << message << "\n";
        return;
    }

    std::string topic = message.substr(0, colonPos);
    std::string data = message.substr(colonPos + 1);

    std::istringstream iss(data);

    if (topic == "CLIENT_CONNECTED") {
        uint32_t clientId;
        float x, y;
        if (iss >> clientId >> x >> y) {
            HandleClientConnected(clientId, x, y);
        } else {
            std::cout << "Invalid CLIENT_CONNECTED format: " << data << "\n";
        }
    } else if (topic == "CLIENT_DISCONNECTED") {
        uint32_t clientId;
        if (iss >> clientId) {
            HandleClientDisconnected(clientId);
        } else {
            std::cout << "Invalid CLIENT_DISCONNECTED format: " << data << "\n";
        }
    } else if (topic == "POSITION_UPDATE") {
        uint32_t clientId;
        float x, y;
        uint64_t timestamp;
        if (iss >> clientId >> x >> y >> timestamp) {
            HandlePositionUpdate(clientId, x, y, timestamp);
        } else {
            std::cout << "Invalid POSITION_UPDATE format: " << data << "\n";
        }
    } else {
        std::cout << "Unknown broadcast topic: " << topic << "\n";
    }
}

void Client::HandleClientConnected(uint32_t clientId, float x, float y) {
    // Don't track ourselves
    if (clientId == this->clientId.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(otherClientsMutex);
        otherClients[clientId] = OtherClientData(x, y);
    }

    std::cout << "Client " << clientId << " connected at position (" << x << ", " << y << ")\n";
}

void Client::HandleClientDisconnected(uint32_t clientId) {
    // Don't process our own disconnect messages
    if (clientId == this->clientId.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(otherClientsMutex);
        auto it = otherClients.find(clientId);
        if (it != otherClients.end()) {
            otherClients.erase(it);
            std::cout << "Client " << clientId << " disconnected\n";
        }
    }
}

void Client::HandlePositionUpdate(uint32_t clientId, float x, float y, uint64_t timestamp) {
    // Don't process our own position updates
    if (clientId == this->clientId.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(otherClientsMutex);
        auto it = otherClients.find(clientId);
        if (it != otherClients.end()) {
            // Update position and timestamp
            it->second.x = x;
            it->second.y = y;
            it->second.lastUpdate = std::chrono::steady_clock::now();
        } else {
            // Client position update for unknown client
            otherClients[clientId] = OtherClientData(x, y);
            std::cout << "Added unknown client " << clientId << " from position update\n";
        }
    }
}

void Client::SendPositionUpdateToServer() {
    if (!reqSocket || !connected.load()) {
        return;
    }

    try {
        // Create position update message
        std::ostringstream oss;
        oss << "POSITION " << clientId.load() << " " << currentX.load() << " " << currentY.load();
        std::string posMsg = oss.str();

        // Send position update via REQ socket
        zmq::message_t request(posMsg.size());
        memcpy(request.data(), posMsg.data(), posMsg.size());

        // Send with short timeout (non-blocking approach)
        if (reqSocket->send(request, zmq::send_flags::dontwait)) {
            // Try to receive response but don't block for it
            zmq::message_t reply;
            auto result = reqSocket->recv(reply, zmq::recv_flags::dontwait);

            if (result) {
                std::string response(static_cast<char*>(reply.data()), reply.size());
                // Server should respond with "OK" for position updates
                if (response != "OK") {
                    std::cout << "Unexpected position update response: " << response << "\n";
                }
            }
        }

    } catch (const zmq::error_t& e) {
        if (e.num() == EAGAIN) {
            // Socket busy, skip this update
            return;
        } else if (e.num() != ETERM) {
            std::cout << "ZMQ error sending position update: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error sending position update: " << e.what() << "\n";
    }
}

}