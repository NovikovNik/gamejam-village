#pragma once

#include "Entity.h"
#include <type_traits>
#include <EventBus/EventBus.h>
#include <Map/Map.h>

namespace World {

// Этот хендлер написал AI, я запутался в метаисториях и шаблонах, прости господи за это
// Прослойка между Entity и EventBus: проверяет, что Entity всё ещё в контейнере, прежде чем вызвать callback
struct EntityEventHandler {
    template <typename TEvent, typename TEntity>
    static void Subscribe(TEntity* entity, void (TEntity::*callback)(TEvent&)) {
        static_assert(std::is_base_of_v<Entity, TEntity>, "TEntity must derive from Entity");
        EventBus::instance().SubscribeToEvent<TEvent>(
            entity,
            callback,
            [entity] { return MapManager::GetEntitiesContainer().Contains(entity); }
        );
    }
};

} // namespace World
