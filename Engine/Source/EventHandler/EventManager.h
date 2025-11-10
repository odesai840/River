#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "Math/Math.h"
#include "Core/Timeline.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>

namespace RiverCore {

enum EventType
{
    EVENT_TYPE_DEATH,
    EVENT_TYPE_SPAWN,
    EVENT_TYPE_COLLISION,
    EVENT_TYPE_INPUT,
    EVENT_TYPE_START_RECORDING,
    EVENT_TYPE_STOP_RECORDING,
    EVENT_TYPE_START_PLAYBACK,
    EVENT_TYPE_STOP_PLAYBACK,
    EVENT_TYPE_CLEAR_REPLAY
};

// Data payload for events
struct EventData {
    uint32_t entityID = 0;              // Primary entity involved
    uint32_t secondaryEntityID = 0;     // For collisions (other entity)
    Vec2 position = Vec2::zero();       // For spawn positions
    int collisionSide = 0;              // Collision direction
    std::unordered_map<std::string, bool> inputButtons;  // For input events
    int keyframeInterval = 1;          // For replay recording (seconds between keyframes)

    // Default constructor
    EventData() = default;

    // Constructor for simple events with just entity ID
    EventData(uint32_t entityID) : entityID(entityID) {}
};

// Event with an int type and handler that handles the event
// type is also used for queueing
struct Event {
    int type;
    std::function<void(EventData)> handler;  // Used for registration only
    EventData data;                          // Used for queueing only
    float timestamp;                         // Used for queueing only

    // Default constructor
    Event() : type(0), timestamp(-1.0f) {}

    // Constructor for registration (with handler)
    Event(int type, std::function<void(EventData)> handler)
        : type(type), handler(handler), timestamp(-1.0f) {}

    // Constructor for queueing (with data and timestamp)
    Event(int type, EventData data, float timestamp)
        : type(type), data(data), timestamp(timestamp) {}

    // Comparison operator for priority queue
    bool operator>(const Event& other) const {
        // First priority: timestamp (earlier = higher priority)
        if (timestamp != other.timestamp) {
            return timestamp > other.timestamp;
        }
        // Second priority: event type (lower enum value = higher priority)
        return type > other.type;
    }
};

class EventManager {
public:
    // Create an event manager
    EventManager() = default;

    // Set timeline reference for timestamp tracking
    void SetTimeline(Timeline* timeline) {
        timelineRef = timeline;
    }

    // Set input recording callback (called when input events are queued during recording)
    void SetInputRecordingCallback(std::function<void(const EventData&)> callback) {
        inputRecordCallback = callback;
    }

    // Registers an event into the event map
    void Register(int type, Event e) {
        eventMap[type].push_back(e);
    }

    // Deregisters the event
    void Deregister(int type) {
        eventMap.erase(type);
    }

    // Pushes event to the queue with data and optional timestamp
    // If timestamp < 0, uses current time (immediate processing)
    // If timestamp >= 0, event is scheduled for that time (delayed event)
    void Queue(int type, EventData data = EventData(), float timestamp = -1.0f);

    // Raises all events whose timestamp has arrived (timestamp <= current time)
    void Raise();

private:
    std::unordered_map<int, std::vector<Event>> eventMap;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;
    std::function<void(const EventData&)> inputRecordCallback;
    Timeline* timelineRef = nullptr;
};

}

#endif
