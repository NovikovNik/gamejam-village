#pragma once

#include "EventBus.h"
#include <queue>
#include <functional>
#include <Utils/Singleton.h>

class EventsQueue: public Singleton<EventsQueue> {
public:
    template <typename TEvent>
    void Push(const TEvent& e) {
        TEvent copy = e;
        events.push([copy]() {
            EventBus::instance().EmitEvent<TEvent>(copy);
        });
    }

    void Dispatch();
    void Clear();

private:
    std::queue<std::function<void()>> events;
};