#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

namespace LocationsStates {
//    using LocationName = std::string;
    struct Object
    {
        std::string name;
        std::string type;

        auto operator<=>(const Object&) const = default;
    };
//    struct LocationChanges
//    {
//        LocationObjects added;
//        LocationObjects removed;
//    };
//    
//    using LocationObjects = std::vector<Object>;
//    using State = std::unordered_map<LocationName, LocationObjects>;

    struct State
    {
        std::map<std::string, bool> registeredLocations;
        std::map<std::string, std::set<std::string>> registeredEntities;
    };

}
