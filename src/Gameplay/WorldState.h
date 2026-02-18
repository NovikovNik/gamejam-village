#pragma once 

#include "LocationsStates.h"

namespace WorldState {
    void Initiate();
    void Destroy();
    [[nodiscard]] LocationsStates::LocationChanges SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName);
}
