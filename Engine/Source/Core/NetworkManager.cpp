#include "NetworkManager.h"
#include <iostream>

namespace RiverCore {

NetworkManager::NetworkManager() {
}

NetworkManager::~NetworkManager() {
    Disconnect();
}

bool NetworkManager::Connect(const std::string& serverAddress) {
    if (!client.Connect(serverAddress)) {
        std::cout << "NetworkManager: Failed to connect to server at " << serverAddress << std::endl;
        return false;
    }
    std::cout << "NetworkManager: Successfully connected to server" << std::endl;
    return true;
}

void NetworkManager::Disconnect() {
    client.Disconnect();

    // Clean up all networked player entities
    if (entityManagerRef) {
        for (auto& [clientId, entityId] : networkedPlayers) {
            entityManagerRef->RemoveEntity(entityId);
        }
    }

    networkedPlayers.clear();
    lastKnownClients.clear();
    localPlayerEntityId = 0;
}

void NetworkManager::Update() {
    if (!IsConnected()) {
        return;
    }

    // Update client networking
    client.Update();

    // Automatic local player position sync
    if (localPlayerEntityId != 0 && entityManagerRef) {
        Entity* localPlayer = entityManagerRef->GetEntityByID(localPlayerEntityId);
        if (localPlayer) {
            client.SendPosition(localPlayer->position.x, localPlayer->position.y);
        }
    }

    // Automatic networked player management
    UpdateNetworkedPlayers();
}

bool NetworkManager::IsConnected() const {
    return client.IsConnected();
}

void NetworkManager::SetLocalPlayer(uint32_t entityId) {
    localPlayerEntityId = entityId;
    std::cout << "NetworkManager: Set local player entity ID to " << entityId << std::endl;
}

void NetworkManager::SendPosition(float x, float y) {
    if (IsConnected()) {
        client.SendPosition(x, y);
    }
}

void NetworkManager::UpdateNetworkedPlayers() {
    if (!entityManagerRef) {
        return;
    }

    const auto& currentClients = client.GetOtherClients();

    // Create new networked players for clients we haven't seen before
    for (const auto& [clientId, clientData] : currentClients) {
        if (networkedPlayers.find(clientId) == networkedPlayers.end()) {
            CreateNetworkedPlayer(clientId, clientData.x, clientData.y);
        }
        else {
            // Update position if it has changed
            auto lastKnownIt = lastKnownClients.find(clientId);
            if (lastKnownIt == lastKnownClients.end() ||
                lastKnownIt->second.x != clientData.x ||
                lastKnownIt->second.y != clientData.y) {

                uint32_t entityId = networkedPlayers[clientId];
                entityManagerRef->SetPosition(entityId, Vec2(clientData.x, clientData.y));

                // Log real-time position updates (can be removed for production)
                static int updateCount = 0;
                if (++updateCount % 60 == 0) { // Log every 60 updates (once per second at 60Hz)
                    std::cout << "NetworkManager: Real-time position update for client " << clientId
                              << " to (" << clientData.x << ", " << clientData.y << ")" << std::endl;
                }
            }
        }
    }

    // Remove networked players for clients that have disconnected
    auto it = networkedPlayers.begin();
    while (it != networkedPlayers.end()) {
        uint32_t clientId = it->first;
        if (currentClients.find(clientId) == currentClients.end()) {
            RemoveNetworkedPlayer(clientId);
            it = networkedPlayers.erase(it);
        } else {
            ++it;
        }
    }

    // Update our cache of last known positions
    lastKnownClients = currentClients;
}

std::unordered_map<uint32_t, OtherClientData> NetworkManager::GetOtherClients() const {
    return client.GetOtherClients();
}

void NetworkManager::CreateNetworkedPlayer(uint32_t clientId, float x, float y) {
    if (!entityManagerRef) {
        return;
    }

    // Create animated entity using same sprite as local player
    uint32_t entityId = entityManagerRef->AddAnimatedEntity(
        "Assets/battie.png", 8, 24.0f, x, y, 0.0f, 2.0f, 2.0f, false);

    // Set as visual-only entity (no physics, no collisions)
    entityManagerRef->SetColliderType(entityId, ColliderType::NONE);

    // Store the mapping
    networkedPlayers[clientId] = entityId;

    std::cout << "NetworkManager: Created networked player entity " << entityId << " for client " << clientId << std::endl;
}

void NetworkManager::RemoveNetworkedPlayer(uint32_t clientId) {
    if (!entityManagerRef) {
        return;
    }

    auto it = networkedPlayers.find(clientId);
    if (it != networkedPlayers.end()) {
        uint32_t entityId = it->second;
        entityManagerRef->RemoveEntity(entityId);
        std::cout << "NetworkManager: Removed networked player entity " << entityId << " for client " << clientId << std::endl;
    }
}

}