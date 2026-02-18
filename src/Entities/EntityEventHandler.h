#pragma once

#include <EventBus/EventBus.h>

// ---------------------------------------------------------------------------
// EntityEventHandler
// Convenience wrapper around EventBus for member-function subscriptions.
//
//   auto sub = EntityEventHandler::Subscribe<MyEvent>(this, &MyClass::OnMyEvent);
// ---------------------------------------------------------------------------
class EntityEventHandler {
public:
    template <typename TEvent, typename TOwner>
    [[nodiscard]] static Events::Handler Subscribe(TOwner* owner, void (TOwner::*method)(TEvent&)) {
        return EventBus::instance().SubscribeToEvent<TEvent>(
            [owner, method](TEvent& e) { (owner->*method)(e); });
    }
};
