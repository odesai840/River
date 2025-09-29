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
    lastKnownX.clear();
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

    // Networked player management
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

                // Check for horizontal movement to update sprite direction
                auto lastXIt = lastKnownX.find(clientId);
                if (lastXIt != lastKnownX.end()) {
                    float previousX = lastXIt->second;
                    float currentX = clientData.x;

                    // Only flip sprite if there's significant horizontal movement
                    float threshold = 0.1f;
                    if (currentX < previousX - threshold) {
                        // Moving left
                        entityManagerRef->FlipSprite(entityId, false, false);
                    } else if (currentX > previousX + threshold) {
                        // Moving right
                        entityManagerRef->FlipSprite(entityId, true, false);
                    }
                }

                // Update position
                entityManagerRef->SetPosition(entityId, Vec2(clientData.x, clientData.y));

                // Update stored X position for next comparison
                lastKnownX[clientId] = clientData.x;
            }
        }
    }

    // Remove networked players for clients that have disconnected
    auto it = networkedPlayers.begin();
    while (it != networkedPlayers.end()) {
        uint32_t clientId = it->first;
        if (currentClients.find(clientId) == currentClients.end()) {
            std::cout << "NetworkManager: Client " << clientId << " no longer in server state, removing entity" << std::endl;
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

    // Initialize last known X position for sprite flipping
    lastKnownX[clientId] = x;

    std::cout << "NetworkManager: Created networked player entity " << entityId << " for client " << clientId
              << " at position (" << x << ", " << y << ")" << std::endl;
}

void NetworkManager::RemoveNetworkedPlayer(uint32_t clientId) {
    if (!entityManagerRef) {
        std::cout << "NetworkManager: Cannot remove networked player - no EntityManager reference" << std::endl;
        return;
    }

    auto it = networkedPlayers.find(clientId);
    if (it != networkedPlayers.end()) {
        uint32_t entityId = it->second;
        entityManagerRef->RemoveEntity(entityId);

        // Clean up last known X position tracking
        lastKnownX.erase(clientId);

        std::cout << "NetworkManager: Successfully removed networked player entity " << entityId << " for disconnected client " << clientId << std::endl;
    } else {
        std::cout << "NetworkManager: Warning - tried to remove non-existent networked player for client " << clientId << std::endl;
    }
}

}