#include "GameInterface.h"
#include "Networking/Server.h"
#include "Networking/ServerInputManager.h"
#include "Networking/NetworkProtocol.h"

namespace RiverCore {

uint32_t GameInterface::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation,
    float Xscale, float Yscale, bool physEnabled)
{
    if(entityManagerRef) {
        return entityManagerRef->AddEntity(spritePath, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

uint32_t GameInterface::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps,
    float Xpos, float Ypos, float rotation, float Xscale, float Yscale, bool physEnabled)
{
    if(entityManagerRef) {
        return entityManagerRef->AddAnimatedEntity(spritePath, totalFrames, fps, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled);
    }
    return 0;
}

void GameInterface::RemoveEntity(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->RemoveEntity(entityID);
    }
}

void GameInterface::UpdateEntityPosition(uint32_t entityID, float newX, float newY) {
    if(entityManagerRef) {
        entityManagerRef->UpdateEntityPosition(entityID, newX, newY);
    }
}

std::vector<std::pair<uint32_t, int>> GameInterface::GetEntityCollisions(uint32_t entityID) {
    if (entityManagerRef) {
        std::vector<std::pair<uint32_t, int>> result;
        bool success = entityManagerRef->GetEntityProperty(entityID, [&result](const Entity& entity){
            result = entity.collider.GetCollisions();
        });
        if (success) {
            return result;
        }
    }
    return {};
}

bool GameInterface::IsKeyPressed(SDL_Scancode key) {
    if(inputRef) {
        return inputRef->IsKeyPressed(key);
    }
    return false;
}

void GameInterface::FlipSprite(uint32_t entityID, bool flipX, bool flipY) {
    if(entityManagerRef) {
        entityManagerRef->FlipSprite(entityID, flipX, flipY);
    }
}

bool GameInterface::GetFlipX(uint32_t entityID) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipX(entityID);
    }
    return false;
}

bool GameInterface::GetFlipY(uint32_t entityID) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipY(entityID);
    }
    return false;
}

bool GameInterface::GetFlipState(uint32_t entityID, bool& flipX, bool& flipY) {
    if(entityManagerRef) {
        return entityManagerRef->GetFlipState(entityID, flipX, flipY);
    }
    flipX = false;
    flipY = false;
    return false;
}

void GameInterface::ToggleFlipX(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->ToggleFlipX(entityID);
    }
}

void GameInterface::ToggleFlipY(uint32_t entityID) {
    if(entityManagerRef) {
        entityManagerRef->ToggleFlipY(entityID);
    }
}

void GameInterface::SetColliderType(uint32_t entityID, ColliderType type) {
    if(entityManagerRef) {
        entityManagerRef->SetColliderType(entityID, type);
    }
}

void GameInterface::SetGravity(float gravity) {
    if (physicsRef){
        physicsRef->SetGravity(gravity);
    }
}

float GameInterface::GetGravity(){
    if (physicsRef){
        return physicsRef->GetGravity();
    }
    return 0.0f;
}

void GameInterface::ApplyForce(uint32_t entityID, float forceX, float forceY) {
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->ApplyForce(*entity, Vec2(forceX, forceY));
        }
    }
}

void GameInterface::ApplyImpulse(uint32_t entityID, float impulseX, float impulseY) {
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->ApplyImpulse(*entity, Vec2(impulseX, impulseY));
        }
    }
}

void GameInterface::SetVelocity(uint32_t entityID, float velX, float velY) {
    if (entityManagerRef && physicsRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            physicsRef->SetVelocity(*entity, Vec2(velX, velY));
        }
    }
}

Vec2 GameInterface::GetVelocity(uint32_t entityID) {
    if (entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            return entity->velocity;
        }
    }
    return Vec2::zero();
}

void GameInterface::SetPosition(uint32_t entityID, float newX, float newY) {
    if (entityManagerRef) {
        entityManagerRef->SetPosition(entityID, Vec2(newX, newY));
    }
}

Vec2 GameInterface::GetPosition(uint32_t entityID) {
    if (entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            return entity->position;
        }
    }
    return Vec2::zero();
}

void GameInterface::ToggleScalingMode() {
    if (rendererRef) {
        rendererRef->ToggleScalingMode();
    }
}

void GameInterface::ToggleDebugCollisions() {
    if (rendererRef) {
        rendererRef->ToggleDebugCollisions();
    }
}

void GameInterface::SetCameraPosition(const Vec2& pos) {
    if (rendererRef) {
        rendererRef->GetCamera().SetPosition(pos);
    }
}

void GameInterface::SetCameraPosition(float x, float y) {
    SetCameraPosition(Vec2(x, y));
}

Vec2 GameInterface::GetCameraPosition() const {
    if (rendererRef) {
        return rendererRef->GetCamera().GetPosition();
    }
    return Vec2::zero();
}

void GameInterface::MoveCamera(const Vec2& delta) {
    if (rendererRef) {
        rendererRef->GetCamera().Move(delta);
    }
}

void GameInterface::MoveCamera(float deltaX, float deltaY) {
    MoveCamera(Vec2(deltaX, deltaY));
}

void GameInterface::SnapCameraToPosition(const Vec2& pos) {
    if (rendererRef) {
        rendererRef->GetCamera().SnapToPosition(pos);
    }
}

void GameInterface::SetCameraZoom(float zoom) {
    if (rendererRef) {
        rendererRef->GetCamera().SetZoom(zoom);
    }
}

float GameInterface::GetCameraZoom() const {
    if (rendererRef) {
        return rendererRef->GetCamera().GetZoom();
    }
    return 1.0f;
}

void GameInterface::SetCameraZoomLimits(float min, float max) {
    if (rendererRef) {
        rendererRef->GetCamera().SetZoomLimits(min, max);
    }
}

void GameInterface::SetCameraBounds(const Vec2& min, const Vec2& max) {
    if (rendererRef) {
        rendererRef->GetCamera().SetBounds(min, max);
    }
}

void GameInterface::SetCameraBounds(float minX, float minY, float maxX, float maxY) {
    SetCameraBounds(Vec2(minX, minY), Vec2(maxX, maxY));
}

void GameInterface::EnableCameraBounds(bool enable) {
    if (rendererRef) {
        rendererRef->GetCamera().EnableBounds(enable);
    }
}

void GameInterface::FollowCameraTarget(const Vec2& targetPos, float smoothing, float deltaTime) {
    if (rendererRef) {
        rendererRef->GetCamera().FollowTarget(targetPos, smoothing, deltaTime);
    }
}

void GameInterface::FollowCameraTarget(float targetX, float targetY, float smoothing, float deltaTime) {
    FollowCameraTarget(Vec2(targetX, targetY), smoothing, deltaTime);
}

void GameInterface::SetCameraDeadZone(float width, float height) {
    if (rendererRef) {
        rendererRef->GetCamera().SetFollowDeadZone(width, height);
    }
}

bool GameInterface::IsCameraInDeadZone(const Vec2& targetPos) const {
    if (rendererRef) {
        return rendererRef->GetCamera().IsInDeadZone(targetPos);
    }
    return false;
}

Vec2 GameInterface::GetVisibleWorldMin() const {
    if (rendererRef) {
        return rendererRef->GetCamera().GetVisibleWorldMin();
    }
    return Vec2::zero();
}

Vec2 GameInterface::GetVisibleWorldMax() const {
    if (rendererRef) {
        return rendererRef->GetCamera().GetVisibleWorldMax();
    }
    return Vec2::zero();
}

bool GameInterface::IsWorldPositionVisible(const Vec2& worldPos, const Vec2& size) const {
    if (rendererRef) {
        return rendererRef->GetCamera().IsVisible(worldPos, size);
    }
    return false;
}

void GameInterface::SetTimeScale(float scale) {
    if (timelineRef) {
        timelineRef->SetTimeScale(scale);
    }
}

float GameInterface::GetTimeScale() const {
    if (timelineRef) {
        return timelineRef->GetTimeScale();
    }
    return 1.0f;
}

void GameInterface::IncreaseTimeScale() {
    if (timelineRef) {
        timelineRef->IncreaseTimeScale();
    }
}

void GameInterface::DecreaseTimeScale() {
    if (timelineRef) {
        timelineRef->DecreaseTimeScale();
    }
}

void GameInterface::SetPaused(bool isPaused) {
    if (timelineRef) {
        timelineRef->SetPaused(isPaused);
    }
}

bool GameInterface::IsPaused() const {
    if (timelineRef) {
        return timelineRef->IsPaused();
    }
    return false;
}

InputState GameInterface::GetInputForClient(uint32_t clientID) {
    if (serverInputManagerRef) {
        return serverInputManagerRef->GetInputForClient(clientID);
    }
    return {};
}

std::vector<uint32_t> GameInterface::GetConnectedClients() {
    if (serverRef) {
        return serverRef->GetConnectedClients();
    }
    return {};
}

uint32_t GameInterface::GetPlayerEntityForClient(uint32_t clientID) {
    if (serverRef) {
        return serverRef->GetPlayerEntityForClient(clientID);
    }
    return 0;
}

void GameInterface::RegisterPlayerEntity(uint32_t clientID, uint32_t entityID) {
    if (serverRef) {
        serverRef->RegisterPlayerEntity(clientID, entityID);
    }
}

void GameInterface::SendInputToServer(const std::unordered_map<std::string, bool>& buttons) {
    if (networkManagerRef) {
        networkManagerRef->SendInput(buttons);
    }
}

uint32_t GameInterface::GetLocalClientId() {
    if (networkManagerRef) {
        return networkManagerRef->GetClientId();
    }
    return 0;
}

uint32_t GameInterface::GetLocalPlayerEntity() {
    if (networkManagerRef) {
        return networkManagerRef->GetLocalPlayerEntity();
    }
    return 0;
}

void GameInterface::Register(int type, Event e)
{
    if (eventManagerRef) {
        eventManagerRef->Register(type, e);
    }
}

void GameInterface::Deregister(int type)
{
    if (eventManagerRef) {
        eventManagerRef->Deregister(type);
    }
}

void GameInterface::Queue(int type, EventData data)
{
    if (eventManagerRef) {
        eventManagerRef->Queue(type, data);
    }
}

void GameInterface::Raise()
{
    if (eventManagerRef) {
        eventManagerRef->Raise();
    }
}

void GameInterface::BroadcastEntitySpawn(uint32_t entityID, uint32_t ownerClientID, uint32_t excludeClientID) {
    if (serverRef && entityManagerRef) {
        Entity* entity = entityManagerRef->GetEntityByID(entityID);
        if (entity) {
            EntitySpawnInfo spawnInfo;
            spawnInfo.entityID = entity->ID;
            spawnInfo.spritePath = entity->spritePath;
            spawnInfo.totalFrames = entity->totalFrames;
            spawnInfo.fps = entity->fps;
            spawnInfo.position = entity->position;
            spawnInfo.scale = entity->scale;
            spawnInfo.rotation = entity->rotation;
            spawnInfo.physEnabled = entity->physApplied;
            spawnInfo.colliderType = static_cast<int>(entity->collider.type);

            serverRef->BroadcastEntitySpawn(spawnInfo, ownerClientID, excludeClientID);
        }
    }
}

void GameInterface::BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID) {
    if (serverRef) {
        serverRef->BroadcastEntityDespawn(entityID, excludeClientID);
    }
}

}