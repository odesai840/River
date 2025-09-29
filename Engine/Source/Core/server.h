#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>

namespace RiverCore {

// Struct for client data
struct ClientData {
    float x = 0.0f;
    float y = 0.0f;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    bool positionChanged = false;

    ClientData() : lastUpdate(std::chrono::steady_clock::now()) {}
    ClientData(float x, float y) : x(x), y(y), lastUpdate(std::chrono::steady_clock::now()) {}
};

class Server {
public:
    Server();
    ~Server();

    // Starts the server
    void Start();
    // Stops the server gracefully
    void Stop();

    // Update a client's position (thread-safe)
    void UpdateClientPosition(uint32_t clientId, float x, float y);
    // Remove a client (thread-safe)
    void RemoveClient(uint32_t clientId);

private:
    // Map of clients and their associated data
    std::unordered_map<uint32_t, ClientData> clientData;
    // Mutex for client data access
    mutable std::mutex clientDataMutex;

    // Next available client ID
    std::atomic<uint32_t> nextClientID{1};

    // Server running state
    std::atomic<bool> running{false};

    // 60hz broadcast interval
    static constexpr int BROADCAST_INTERVAL_MS = 16;

    // Processes server connections
    void ProcessConnectionRequests();
    // Broadcasts game state to connected clients
    void BroadcastGameState();

    // Handles connecting clients to the server
    uint32_t HandleConnect();
    // Handles disconnecting clients from the server
    void HandleDisconnect(uint32_t clientId);

    // Broadcasts new connections to clients
    void BroadcastClientConnected(uint32_t clientId, float x, float y);
    // Broadcasts disconnections to clients
    void BroadcastClientDisconnected(uint32_t clientId);
    // Broadcasts client position updates to clients
    void BroadcastPositionUpdate(uint32_t clientId, float x, float y);
    // Broadcasts server messages
    void BroadcastMessage(const std::string& message);

    // Initializes sockets
    void InitializeSockets();
    // Cleans up sockets
    void CleanupSockets();
};

}

#endif