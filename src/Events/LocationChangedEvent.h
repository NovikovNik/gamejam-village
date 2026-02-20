#pragma once

#include "Event.h"
#include <string>

class LocationChangedEvent : public Event {
public:
    std::string locationName;

    LocationChangedEvent(std::string locationName) : locationName {std::move(locationName)} {}
};
