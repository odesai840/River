#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "Math/Math.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <utility>

namespace RiverCore {

enum EventType
{
    EVENT_TYPE_DEATH,
    EVENT_TYPE_SPAWN,
    EVENT_TYPE_COLLISION,
    EVENT_TYPE_INPUT
};

// Data payload for events
struct EventData {
    uint32_t entityID = 0;              // Primary entity involved
    uint32_t secondaryEntityID = 0;     // For collisions (other entity)
    Vec2 position = Vec2::zero();       // For spawn positions
    int collisionSide = 0;              // Collision direction
    std::unordered_map<std::string, bool> inputButtons;  // For input events

    // Default constructor
    EventData() = default;

    // Constructor for simple events with just entity ID
    EventData(uint32_t entityID) : entityID(entityID) {}
};

// Event with an int type and handler that handles the event
// type is also used for queueing
struct Event {
    int type;
    std::function<void(EventData)> handler;

    // Create an empty event
    Event() = default;
    //Create an event with specific type and function to handle
    Event(int type, std::function<void(EventData)> handler): type(type), handler(handler) {}

    // Changes the function of the event
    void ChangeHandler(Event e, std::function<void(EventData)> newHandler) {
        e.handler = newHandler;
    }

    // Changes the type of event
    void ChangeType(Event e, int newType) {
        e.type = newType;
    }

};

class EventManager {
public:
    // Create an event manager
    EventManager() = default;

    // Registers an event into the event map
    void Register(int type, Event e) {
        eventMap[type].push_back(e);
    }

    // Deregisters the event
    void Deregister(int type) {
        eventMap.erase(type);
    }

    // Pushes event to the queue with data
    void Queue(int type, EventData data = EventData()) {
        eventQueue.push({type, data});
    }

    void Raise() {

        int eventsToProcess = eventQueue.size();

        for (int i = 0; i < eventsToProcess; i++) {

            auto eventPair = eventQueue.front();
            eventQueue.pop();

            int eventType = eventPair.first;
            EventData eventData = eventPair.second;

            auto it = eventMap.find(eventType);
            if ( it != eventMap.end()) {
                for (auto& event : it->second) {
                    event.handler(eventData);
                }
            }
        }
    }
private:
    std::unordered_map<int, std::vector<Event>> eventMap;
    std::queue<std::pair<int, EventData>> eventQueue;
};

}

#endif