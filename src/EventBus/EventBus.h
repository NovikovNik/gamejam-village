#pragma once

#include "EventsHandler.h"
#include "../Logger/Logger.h"
#include <Utils/Singleton.h>
#include <typeindex>
#include <map>
#include <list>
#include <functional>
#include <memory>

// ---------------------------------------------------------------------------
// Type-erased base — one per subscription
// ---------------------------------------------------------------------------
class IEventCallback {
public:
    explicit IEventCallback(int32_t id) : id(id) {}
    virtual ~IEventCallback() = default;

    virtual void Execute(void* event) = 0;
    int32_t GetId() const { return id; }

private:
    int32_t id;
};

// ---------------------------------------------------------------------------
// Typed callback — TEvent is the event type, callback receives TEvent&
// ---------------------------------------------------------------------------
template <typename TEvent>
class EventCallback : public IEventCallback {
public:
    using Fn = std::function<void(TEvent&)>;

    EventCallback(Fn fn, int32_t id)
        : IEventCallback(id)
        , fn(std::move(fn))
    {}

    void Execute(void* event) override {
        fn(*static_cast<TEvent*>(event));
    }

private:
    Fn fn;
};

using HandlerList = std::list<std::unique_ptr<IEventCallback>>;

// ---------------------------------------------------------------------------
// EventBus
// ---------------------------------------------------------------------------
class EventBus : public Singleton<EventBus> {
public:
    EventBus()  { Logger::Log("EventBus constructor called"); }
    ~EventBus() { Logger::Log("EventBus destructor called"); }

    void Reset() { subscribers.clear(); }

    // Subscribe to TEvent. Keep the returned Handler alive to stay subscribed.
    //
    //   auto sub = EventBus::instance().SubscribeToEvent<MyEvent>(
    //       [this](MyEvent& e) { ... });
    template <typename TEvent>
    [[nodiscard]] Events::Handler SubscribeToEvent(std::function<void(TEvent&)> callback) {
        auto& list = subscribers[typeid(TEvent)];
        if (!list) {
            list = std::make_unique<HandlerList>();
        }
        Events::Handler h;
        h.Initialize();
        list->push_back(std::make_unique<EventCallback<TEvent>>(std::move(callback), h.GetId()));
        return h;
    }

    template <typename TEvent, typename TOwner>
    [[nodiscard]] Events::Handler SubscribeToEvent(TOwner* owner, void (TOwner::*method)(TEvent&)) {
        return SubscribeToEvent<TEvent>(
            [owner, method](TEvent& e) { (owner->*method)(e); });
    }

    // Emit TEvent — construct it from args and dispatch to all subscribers.
    //
    //   EventBus::instance().EmitEvent<MyEvent>(arg1, arg2);
    template <typename TEvent, typename ...TArgs>
    void EmitEvent(TArgs&&... args) {
        auto it = subscribers.find(typeid(TEvent));
        if (it == subscribers.end() || !it->second) {
            return;
        }

        TEvent event(std::forward<TArgs>(args)...);
        for (auto& handler : *it->second) {
            handler->Execute(&event);
        }
    }

    // Called automatically by Events::Handler's destructor
    void Unsubscribe(int32_t id) {
        for (auto& [typeIdx, handlers] : subscribers) {
            if (handlers) {
                handlers->remove_if([id](const std::unique_ptr<IEventCallback>& cb) {
                    return cb->GetId() == id;
                });
            }
        }
    }

private:
    std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;
};
