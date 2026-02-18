#include "EventsQueue.h"
#include "EventBus.h"

void EventsQueue::Dispatch() {
    while (!events.empty()) {
        events.front()();
        events.pop();
    }
}

void EventsQueue::Clear() {
    while (!events.empty()) {
        events.pop();
    }
}