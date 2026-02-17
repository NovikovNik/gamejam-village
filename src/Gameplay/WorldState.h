#pragma once 

#include "LocationsStates.h"

namespace WorldState {
    void Initiate();
    [[nodiscard]] LocationsStates::LocationChanges SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName);
}
