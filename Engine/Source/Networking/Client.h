#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>

namespace RiverCore {

// Struct for client data (other clients)
struct OtherClientData {
    float x = 0.0f;
    float y = 0.0f;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;

    OtherClientData() : lastUpdate(std::chrono::steady_clock::now()) {}
    OtherClientData(float x, float y) : x(x), y(y), lastUpdate(std::chrono::steady_clock::now()) {}
};

class Client {
public:
    Client();
    ~Client();

    // Connect to server
    bool Connect(const std::string& serverAddress);
    // Disconnect from server gracefully
    void Disconnect();
    // Update client networking
    void Update();

    // Send position update (thread-safe)
    void SendPosition(float x, float y);
    // Get other clients' positions (thread-safe)
    std::unordered_map<uint32_t, OtherClientData> GetOtherClients() const;

    // Check if connected to server
    bool IsConnected() const { return connected.load() && clientId.load() != 0; }
    // Get this client's ID
    uint32_t GetClientId() const { return clientId.load(); }

private:
    // Thread-safe connection state
    std::atomic<bool> connected{false};
    std::atomic<uint32_t> clientId{0};

    // Thread-safe client data management
    std::unordered_map<uint32_t, OtherClientData> otherClients;
    mutable std::mutex otherClientsMutex;

    // Thread-safe position tracking
    std::atomic<float> currentX{0.0f};
    std::atomic<float> currentY{0.0f};
    std::atomic<bool> positionDirty{false};

    // Connection timing
    std::chrono::time_point<std::chrono::steady_clock> lastPositionUpdate;
    static constexpr int POSITION_UPDATE_INTERVAL_MS = 16;

    // Message parsing and processing
    void ProcessIncomingMessages();
    void ParseBroadcastMessage(const std::string& message);
    void HandleClientConnected(uint32_t clientId, float x, float y);
    void HandleClientDisconnected(uint32_t clientId);
    void HandlePositionUpdate(uint32_t clientId, float x, float y, uint64_t timestamp);

    // Socket management
    void InitializeSockets(const std::string& serverAddress);
    void CleanupSockets();

    // Position update sending
    void SendPositionUpdateToServer();
};

}

#endif
