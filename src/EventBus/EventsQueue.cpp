#include "EventsQueue.h"
#include "EventBus.h"

void EventsQueue::Dispatch() {
    while (!events.empty()) {
        events.front()();
        events.pop();
    }
}