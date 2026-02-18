#pragma once

#include "../Logger/Logger.h"
#include "../Events/Event.h"
#include <Utils/Singleton.h>
#include <typeindex>
#include <map>
#include <list>
#include <functional>
#include <memory>

class IEventCallback {
    private:
        virtual void Call(Event& e) = 0;
    public:
        virtual ~IEventCallback() = default;
        void Execute(Event& e) {
            Call(e);
        }
        virtual bool IsAlive() const = 0;
        virtual void* GetOwner() const = 0;
};

template <typename TOwner,typename TEvent>
class EventCallback: public IEventCallback {
    private:
        typedef void (TOwner::*CallbackFunction)(TEvent&);

        TOwner* ownerInstance;
        CallbackFunction callbackFunction;

        virtual void Call(Event& e) override {
            std::invoke(callbackFunction, ownerInstance, static_cast<TEvent&>(e));
        }
        virtual bool IsAlive() const override {
            return ownerInstance != nullptr;
        }
        void* GetOwner() const override {
            return ownerInstance;
        }

    public:
        EventCallback(
            TOwner* ownerInstance,
            CallbackFunction callbackFunction):
            ownerInstance(ownerInstance),
            callbackFunction(callbackFunction){};

        virtual ~EventCallback() override = default;
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus: public Singleton<EventBus>  {
    private:
        std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;
    public:
        EventBus() {
            Logger::Log("EventBus constructor called");
        };
        ~EventBus() {
            Logger::Log("EventBus destructor called");
        };

        // Clear all subscribers (resetting event bus)
        void Reset() {
            subscribers.clear();
        }

        // Remove all subscriptions for the given owner (call from OnDestroy)
        [[maybe_unused]] void UnsubscribeAll(void* owner) {
            for (auto& [typeIdx, handlers] : subscribers) {
                if (handlers) {
                    handlers->remove_if([owner](const std::unique_ptr<IEventCallback>& cb) {
                        return cb->GetOwner() == owner;
                    });
                }
            }
        }

        // eventBus->SubscribeToEvent<CollisionEvent, CollisionSystem>(this, &CollisionSystem::OnCollision);
        template <typename TEvent, typename TOwner>
        void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent&)) {
            if (!subscribers[typeid(TEvent)].get()) {
                subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
            }

            auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
            subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
        };

        template <typename TEvent, typename ...TArgs>
        void EmitEvent(TArgs&& ...args) {
            auto handlers = subscribers[typeid(TEvent)].get();
            if (handlers) {
                for (auto it = handlers->begin(); it != handlers->end(); it++) {
                    if (!it->get()->IsAlive()) {
                        it = handlers->erase(it); // Чистим подписки
                        continue;
                    }
                    auto handler = it->get();
                    TEvent event(std::forward<TArgs>(args)...);
                    handler->Execute(event);
                }
            }
        };
};
