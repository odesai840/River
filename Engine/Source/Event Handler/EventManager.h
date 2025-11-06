#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <SDL3/SDL.h>
#include <functional>    // For std::function, used for event handlers
#include <unordered_map> // For std::unordered_map, to store event listeners
#include <vector>
#include <string>
#include <queue>
#include <utility>

namespace RiverCore{

enum EventType
{
    EVENT_TYPE_DEATH,
    EVENT_TYPE_SPAWN,
    EVENT_TYPE_COLLISION,
    EVENT_TYPE_INPUT
};

// Event with an int type and handler that handles the event
// type is also used for queueing
struct Event {
    int type;
    std::function<void()> handler;

    // Create an empty event
    Event() = default;
    //Create an event with specific type and function to handle
    Event(int type, std::function<void()> handler): type(type), handler(handler) {};

    // Changes the function of the event
    void ChangeHandler(Event e, std::function<void()> newHandler) {
        e.handler = newHandler;
    };

    // Changes the type of event
    void ChangeType(Event e, int newType) {
        e.type = newType;
    };

};

class EventManager {
public:
    // Create an event manager
    EventManager() = default;
    
    // Registers an event into the event map
    void Register(std::string name, Event e) {
        eventMap.insert(std::make_pair(name, e));
    }

    // Deregisters the event
    void Deregister(std::string name) {
        eventMap.erase(name);
    }

    // Pushes event to the queue
    void Queue(std::string name) {
        eventQueue.push(name);
    }

    void Raise(Event e) {

        int eventsToProcess = eventQueue.size();

        for (int i = 0; i < eventsToProcess; i++) {

            std::string eventName = eventQueue.front();
            eventQueue.pop();

            auto it = eventMap.find(eventName);
            if ( it != eventMap.end()) {
                for (auto& handler : it->second) {
                    handler();
                }
            }
        }
    }
private:
    std::unordered_map<std::string, Event> eventMap;
    std::queue<std::string> eventQueue;
};

}

#endif