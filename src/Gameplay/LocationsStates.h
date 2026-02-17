#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace LocationsStates {
    using LocationName = std::string;
    struct Object
    {
        std::string name;
        std::string type;

        auto operator<=>(const Object&) const = default;
    };
    using LocationObjects = std::vector<Object>;
    using State = std::unordered_map<LocationName, LocationObjects>;

    struct LocationChanges
    {
        LocationObjects added;
        LocationObjects removed;
    };
}
