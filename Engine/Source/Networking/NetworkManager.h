#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "Client.h"
#include "Renderer/EntityManager.h"
#include <unordered_map>
#include <string>

namespace RiverCore {

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Connects the client to a server
    bool Connect(const std::string& serverAddress);
    // Disconnects the client from a server
    void Disconnect();
    // Updates the local client
    void Update();
    // Returns if the local client is connected to a server
    bool IsConnected() const;

    // Set EntityManager reference for entity manipulation
    void SetEntityManager(EntityManager* entityManager) { entityManagerRef = entityManager; }

    // Sets the entity ID for the local player
    void SetLocalPlayer(uint32_t entityId);
    // Networked player position update
    void SendPosition(float x, float y);

    // Updates the game world for the local player
    void UpdateNetworkedPlayers();
    // Returns data for other clients
    std::unordered_map<uint32_t, OtherClientData> GetOtherClients() const;

private:
    // Client instance for server communication
    Client client;

    // EntityManager reference for creating/updating networked entities
    EntityManager* entityManagerRef = nullptr;

    // Local player tracking
    uint32_t localPlayerEntityId = 0;

    // Map of networked player IDs and their associated entity IDs
    std::unordered_map<uint32_t, uint32_t> networkedPlayers;
    // Map of player IDs and their associated data
    std::unordered_map<uint32_t, OtherClientData> lastKnownClients;

    // Creates a new networked player entity
    void CreateNetworkedPlayer(uint32_t clientId, float x, float y);
    // Removes the entity for a networked player
    void RemoveNetworkedPlayer(uint32_t clientId);
};

}

#endif