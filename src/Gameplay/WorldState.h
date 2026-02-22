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

    bool RegisterInWorldState(const World::Entity::TagName& tagName);
    void RemoveFromWorldState(const World::Entity::TagName& tagName);

    void SetCurrentState(const LocationsStates::State& state);
    [[nodiscard]] const LocationsStates::State& GetCurrentState();
    void SetBackupLocationPath(const std::string& locationPath);
    [[nodiscard]] const std::string& GetBackupLocationPath();
    [[nodiscard]] const std::map<std::string, std::set<std::string>>& GetRegisteredEntities();

}
