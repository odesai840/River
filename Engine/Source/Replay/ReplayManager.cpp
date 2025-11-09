#include "ReplayManager.h"
#include "Renderer/EntityManager.h"
#include "Renderer/Entity.h"
#include <algorithm>
#include <iostream>

namespace RiverCore {

ReplayManager::ReplayManager() {
}

void ReplayManager::StartRecording(float keyframeInterval) {
    if (recording || playing) {
        std::cout << "[ReplayManager] Cannot start recording: already recording or playing" << std::endl;
        return;
    }

    // Clear any existing replay data
    ClearReplay();

    // Set recording parameters
    recording = true;
    keyframeIntervalSeconds = keyframeInterval;
    recordingTime = 0.0f;
    timeSinceLastKeyframe = 0.0f;

    // Capture initial game state
    CaptureKeyframe();

    std::cout << "[ReplayManager] Recording started (keyframe interval: " << keyframeIntervalSeconds << " seconds)" << std::endl;
}

void ReplayManager::StopRecording() {
    if (!recording) {
        return;
    }

    recording = false;

    std::cout << "[ReplayManager] Recording stopped. Captured " << keyframes.size()
              << " keyframes and " << inputEvents.size() << " input events over "
              << recordingTime << " seconds" << std::endl;
}

void ReplayManager::StartPlayback() {
    if (playing || recording) {
        std::cout << "[ReplayManager] Cannot start playback: already playing or recording" << std::endl;
        return;
    }

    if (!HasReplay()) {
        std::cout << "[ReplayManager] Cannot start playback: no replay data available" << std::endl;
        return;
    }

    playing = true;
    playbackTime = 0.0f;
    nextInputIndex = 0;

    // Restore initial state
    if (!keyframes.empty()) {
        RestoreKeyframe(keyframes[0]);
    }

    std::cout << "[ReplayManager] Playback started. Total duration: " << GetTotalDuration() << " seconds" << std::endl;
}

void ReplayManager::StopPlayback() {
    if (!playing) {
        return;
    }

    playing = false;
    playbackTime = 0.0f;
    nextInputIndex = 0;

    std::cout << "[ReplayManager] Playback stopped" << std::endl;
}

void ReplayManager::ClearReplay() {
    keyframes.clear();
    inputEvents.clear();
    recordingTime = 0.0f;
    playbackTime = 0.0f;
    nextInputIndex = 0;
    timeSinceLastKeyframe = 0.0f;
}

void ReplayManager::RecordInput(const EventData& inputData) {
    if (!recording) {
        return;
    }

    // Record this input event with current recording time
    inputEvents.emplace_back(recordingTime, inputData);
}

void ReplayManager::Update(EventManager& eventManager, float deltaTime) {
    if (recording) {
        // Increment recording time
        recordingTime += deltaTime;
        timeSinceLastKeyframe += deltaTime;

        // Check if we need to capture a keyframe
        if (timeSinceLastKeyframe >= keyframeIntervalSeconds) {
            CaptureKeyframe();
            timeSinceLastKeyframe = 0.0f;
        }
    }
    else if (playing) {
        // Increment playback time
        playbackTime += deltaTime;

        // Inject all input events that should occur at or before current playback time
        while (nextInputIndex < inputEvents.size() &&
               inputEvents[nextInputIndex].timestamp <= playbackTime) {
            // Queue the input event for this time
            eventManager.Queue(EVENT_TYPE_INPUT, inputEvents[nextInputIndex].inputData);
            nextInputIndex++;
        }

        // Check if we've reached the end of the replay
        float totalDuration = GetTotalDuration();
        if (playbackTime >= totalDuration) {
            std::cout << "[ReplayManager] Reached end of replay" << std::endl;
            StopPlayback();
        }
    }
}

float ReplayManager::GetTotalDuration() const {
    if (keyframes.empty() && inputEvents.empty()) {
        return 0.0f;
    }

    // The total duration is the last keyframe's timestamp
    // or the last input event's timestamp, whichever is greater
    float lastKeyframeTime = keyframes.empty() ? 0.0f : keyframes.back().timestamp;
    float lastInputTime = inputEvents.empty() ? 0.0f : inputEvents.back().timestamp;

    return std::max(lastKeyframeTime, lastInputTime);
}

void ReplayManager::CaptureKeyframe() {
    if (!entityManager) {
        std::cout << "[ReplayManager] Warning: Cannot capture keyframe, EntityManager not set" << std::endl;
        return;
    }

    // Capture current game state
    GameStateSnapshot snapshot = CaptureGameState();

    // Store as keyframe with current recording time
    keyframes.emplace_back(recordingTime, snapshot);
}

void ReplayManager::RestoreKeyframe(const ReplayKeyframe& keyframe) {
    if (!entityManager) {
        std::cout << "[ReplayManager] Warning: Cannot restore keyframe, EntityManager not set" << std::endl;
        return;
    }

    RestoreGameState(keyframe.snapshot);
}

GameStateSnapshot ReplayManager::CaptureGameState() {
    GameStateSnapshot snapshot;
    snapshot.timestamp = static_cast<uint64_t>(recordingTime * 1000.0f);

    // Get all entities from the EntityManager
    std::vector<Entity> entities = entityManager->GetEntitiesCopy();

    // Convert each entity to an EntitySnapshot
    for (const Entity& entity : entities) {
        EntitySnapshot entitySnap;
        entitySnap.entityID = entity.ID;
        entitySnap.position = entity.position;
        entitySnap.velocity = entity.velocity;
        entitySnap.scale = entity.scale;
        entitySnap.rotation = entity.rotation;
        entitySnap.flipX = entity.flipX;
        entitySnap.flipY = entity.flipY;
        entitySnap.currentFrame = entity.currentFrame;

        snapshot.entities.push_back(entitySnap);
    }

    return snapshot;
}

void ReplayManager::RestoreGameState(const GameStateSnapshot& snapshot) {
    // Restore each entity's state from the snapshot
    for (const EntitySnapshot& entitySnap : snapshot.entities) {
        if (entityManager->EntityExists(entitySnap.entityID)) {
            // Update entity state
            entityManager->SetPosition(entitySnap.entityID, entitySnap.position);
            entityManager->FlipSprite(entitySnap.entityID, entitySnap.flipX, entitySnap.flipY);

            // Get entity and update velocity directly
            Entity* entity = entityManager->GetEntityByID(entitySnap.entityID);
            if (entity) {
                entity->velocity = entitySnap.velocity;
                entity->rotation = entitySnap.rotation;
                entity->scale = entitySnap.scale;
                entity->currentFrame = entitySnap.currentFrame;
            }
        }
    }
}

const ReplayKeyframe* ReplayManager::FindNearestKeyframe(float time) const {
    if (keyframes.empty()) {
        return nullptr;
    }

    // Find the keyframe at or before the given time
    const ReplayKeyframe* nearest = nullptr;

    for (const auto& keyframe : keyframes) {
        if (keyframe.timestamp <= time) {
            nearest = &keyframe;
        } else {
            break;
        }
    }

    return nearest;
}

}
