#pragma once

#include "Event.h"
#include <string>

class ChangeLocationEvent : public Event {
public:
    std::string locationPath;

    ChangeLocationEvent(std::string path) : locationPath(std::move(path)) {}
};
