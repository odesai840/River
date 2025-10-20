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
        std::cout << "NetworkManager: Failed to connect to server\n";
        return false;
    }
    std::cout << "NetworkManager: Connected to server\n";
    return true;
}

void NetworkManager::Disconnect() {
    client.Disconnect();
    serverToLocalEntityMap.clear();
    entitySpriteInfo.clear();
}

void NetworkManager::Update() {
    if (!IsConnected()) {
        return;
    }

    // Update client networking
    client.Update();

    // Process entity spawn/despawn messages
    ProcessPendingSpawns();
    ProcessPendingDespawns();

    // Get latest game state from server
    GameStateSnapshot snapshot = client.GetLatestGameState();

    // Synchronize local entities with server state
    if (!snapshot.entities.empty()) {
        SyncEntitiesFromServer(snapshot);
    }
}

bool NetworkManager::IsConnected() const {
    return client.IsConnected();
}

void NetworkManager::SendInput(const std::unordered_map<std::string, bool>& buttons,
                                const std::unordered_map<std::string, float>& axes) {
    if (IsConnected()) {
        client.SendInput(buttons, axes);
    }
}

uint32_t NetworkManager::GetClientId() const {
    return client.GetClientId();
}

void NetworkManager::ProcessPendingSpawns() {
    if (!entityManagerRef) {
        return;
    }

    // Get spawn messages from client
    std::vector<EntitySpawnInfo> spawns = client.GetPendingSpawns();

    for (const EntitySpawnInfo& spawnInfo : spawns) {
        // Check if entity already exists
        if (entityManagerRef->EntityExists(spawnInfo.entityID)) {
            continue;  // Already spawned
        }

        // Create entity based on whether it's animated or not
        uint32_t entityID = 0;
        if (spawnInfo.totalFrames > 1) {
            // Animated entity
            entityID = entityManagerRef->AddAnimatedEntity(
                spawnInfo.spritePath.c_str(),
                spawnInfo.totalFrames,
                spawnInfo.fps,
                spawnInfo.position.x,
                spawnInfo.position.y,
                spawnInfo.rotation,
                spawnInfo.scale.x,
                spawnInfo.scale.y,
                spawnInfo.physEnabled
            );
        } else {
            // Static entity
            entityID = entityManagerRef->AddEntity(
                spawnInfo.spritePath.c_str(),
                spawnInfo.position.x,
                spawnInfo.position.y,
                spawnInfo.rotation,
                spawnInfo.scale.x,
                spawnInfo.scale.y,
                spawnInfo.physEnabled
            );
        }

        // Set collider type
        if (entityID != 0) {
            entityManagerRef->SetColliderType(entityID, static_cast<ColliderType>(spawnInfo.colliderType));
            spawnedEntities.insert(entityID);

            // Check if this entity is owned by us
            if (spawnInfo.ownerClientID == GetClientId() && spawnInfo.ownerClientID != 0) {
                localPlayerEntityId = entityID;
                std::cout << "NetworkManager: This is our player entity! ID: " << entityID << "\n";
            }

            std::cout << "NetworkManager: Spawned entity ID " << entityID
                      << " (" << spawnInfo.spritePath << ")\n";
        }
    }
}

void NetworkManager::ProcessPendingDespawns() {
    if (!entityManagerRef) {
        return;
    }

    // Get despawn messages from client
    std::vector<uint32_t> despawns = client.GetPendingDespawns();

    for (uint32_t entityID : despawns) {
        // Check if we spawned this entity
        if (spawnedEntities.find(entityID) != spawnedEntities.end()) {
            entityManagerRef->RemoveEntity(entityID);
            spawnedEntities.erase(entityID);

            std::cout << "NetworkManager: Despawned entity ID " << entityID << "\n";
        }
    }
}

void NetworkManager::SyncEntitiesFromServer(const GameStateSnapshot& snapshot) {
    if (!entityManagerRef) {
        return;
    }

    // Update all entities from server snapshot
    for (const EntitySnapshot& entitySnap : snapshot.entities) {
        // Check if entity exists locally (by server ID)
        Entity* entity = entityManagerRef->GetEntityByID(entitySnap.entityID);

        if (entity) {
            // Update entity transform from server
            entity->position = entitySnap.position;
            entity->velocity = entitySnap.velocity;
            entity->scale = entitySnap.scale;
            entity->rotation = entitySnap.rotation;
            entity->flipX = entitySnap.flipX;
            entity->flipY = entitySnap.flipY;
            entity->currentFrame = entitySnap.currentFrame;
        }
    }
}

}
