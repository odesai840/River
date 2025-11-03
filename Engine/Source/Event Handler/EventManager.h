#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <SDL3/SDL.h>
#include <functional>    // For std::function, used for event handlers
#include <unordered_map> // For std::unordered_map, to store event listeners
#include <vector>        
#include <string>      


namespace RiverCore{

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
            //List of event types by default the dev can make
            const static int EVENT_TYPE_COLLISION = 3;
            const static int EVENT_TYPE_SPAWN = 2;
            const static int EVENT_TYPE_DEATH = 1;
            const static int EVENT_TYPE_INPUT = 4;

            EventManager() = default;
            void Register();
            void Deregister();
            void Raise();
        private:
            std::unordered_map<std::string, Event> eventMap;
    };
}

#endif