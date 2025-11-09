#include "EventManager.h"

namespace RiverCore {

void EventManager::Queue(int type, EventData data) {
    eventQueue.push({type, data});

    // If callback is set and this is an input event, call it
    if (inputRecordCallback && type == EVENT_TYPE_INPUT) {
        inputRecordCallback(data);
    }
}

}
