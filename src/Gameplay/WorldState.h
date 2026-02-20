#pragma once 

#include "LocationsStates.h"
#include <Entities/Entity.h>

namespace WorldState {
    void Initiate();
    void Destroy();
    [[nodiscard]] LocationsStates::LocationChanges SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName);
    [[nodiscard]] const LocationsStates::State& GetCurrentState();
    [[nodiscard]] const LocationsStates::State& GetLastSeenState();

    void AddToWorldState(const World::Entity::TagName& tagName);
    void RemoveFromWorldState(const World::Entity::TagName& tagName);
}
