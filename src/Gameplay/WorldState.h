#pragma once 

#include "LocationsStates.h"
#include <Entities/Entity.h>
#include <map>
#include <set>
#include <string>
#include <cstdint>

namespace WorldState {
    void Initiate();
    void Destroy();
    void Update();

    void RegisterInWorldState(const World::Entity::TagName& tagName);

    [[nodiscard]] const LocationsStates::State& GetCurrentState();
}
