#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include "EventHandler/EventManager.h"
#include "Networking/NetworkProtocol.h"
#include "Renderer/EntityManager.h"
#include <vector>
#include <memory>

namespace RiverCore {

// Replay frame containing input event data
struct ReplayInputFrame {
    float timestamp = 0.0f;  // Time in seconds when this input occurred
    EventData inputData;

    ReplayInputFrame() = default;
    ReplayInputFrame(float time, const EventData& data)
        : timestamp(time), inputData(data) {}
};

// Replay keyframe containing full game state
struct ReplayKeyframe {
    float timestamp = 0.0f;  // Time in seconds when this keyframe was captured
    GameStateSnapshot snapshot;

    ReplayKeyframe() = default;
    ReplayKeyframe(float time, const GameStateSnapshot& state)
        : timestamp(time), snapshot(state) {}
};

// Manages recording and playback of game replays
class ReplayManager {
public:
    ReplayManager();
    ~ReplayManager() = default;

    // Set reference to EntityManager
    void SetEntityManager(EntityManager* entityMgr) { entityManager = entityMgr; }

    // Recording controls
    void StartRecording(float keyframeIntervalSeconds = 1.0f);
    void StopRecording();
    bool IsRecording() const { return recording; }

    // Playback controls
    void StartPlayback();
    void StopPlayback();
    bool IsPlaying() const { return playing; }

    // Check if a replay is available
    bool HasReplay() const { return !keyframes.empty(); }

    // Clear current replay data
    void ClearReplay();

    // Record an input event
    void RecordInput(const EventData& inputData);

    // Update playback state
    void Update(EventManager& eventManager, float deltaTime);

    // Get current playback time
    float GetCurrentTime() const { return playbackTime; }

    // Get total duration of replay in seconds
    float GetTotalDuration() const;

private:
    // Reference to EntityManager for capturing/restoring state
    EntityManager* entityManager = nullptr;

    // Recording state
    bool recording = false;
    float keyframeIntervalSeconds = 1.0f;  // Capture keyframe every N seconds
    float timeSinceLastKeyframe = 0.0f;    // Time accumulator for keyframes

    // Playback state
    bool playing = false;
    float playbackTime = 0.0f;             // Current playback time in seconds
    size_t nextInputIndex = 0;             // Index of next input to play

    // Current recording time
    float recordingTime = 0.0f;

    // Replay data storage
    std::vector<ReplayKeyframe> keyframes;      // Full game state snapshots
    std::vector<ReplayInputFrame> inputEvents;  // Input events between keyframes

    // Helper methods
    void CaptureKeyframe();
    void RestoreKeyframe(const ReplayKeyframe& keyframe);
    GameStateSnapshot CaptureGameState();
    void RestoreGameState(const GameStateSnapshot& snapshot);

    // Find the nearest keyframe at or before a given time
    const ReplayKeyframe* FindNearestKeyframe(float time) const;
};

}

#endif
