#include "EventManager.h"

namespace RiverCore {

void EventManager::Queue(int type, EventData data, float timestamp) {
    // Determine the actual timestamp for this event
    float eventTime = timestamp;
    
    // If timestamp < 0, use current time (immediate processing)
    if (timestamp < 0.0f && timelineRef) {
        eventTime = timelineRef->GetCurrentTime();
    } else if (timestamp < 0.0f) {
        eventTime = 0.0f;
    }

    // Create queued event and add to priority queue
    eventQueue.emplace(type, data, eventTime);

    // If callback is set and this is an input event, call it
    if (inputRecordCallback && type == EVENT_TYPE_INPUT) {
        inputRecordCallback(data);
    }
}

void EventManager::Raise() {
    // Get current time to determine which events should be processed
    float currentTime = timelineRef ? timelineRef->GetCurrentTime() : 0.0f;

    // Process all events whose timestamp has arrived
    while (!eventQueue.empty()) {
        const Event& nextEvent = eventQueue.top();

        // Check if this event should be processed yet
        if (nextEvent.timestamp > currentTime) {
            // This event and all remaining events are in the future
            break;
        }

        // Extract event information
        int eventType = nextEvent.type;
        EventData eventData = nextEvent.data;

        // Remove event from queue
        eventQueue.pop();

        // Find and invoke all handlers for this event type
        auto it = eventMap.find(eventType);
        if (it != eventMap.end()) {
            for (auto& event : it->second) {
                event.handler(eventData);
            }
        }
    }
}

}
