#pragma once

#include "Event.h"
#include <string>

class ChangeLocationEvent : public Event {
public:
    std::string locationPath;
    std::string spawnPoint;

    ChangeLocationEvent(std::string path, std::string spawnPoint = "") : locationPath(std::move(path)), spawnPoint(std::move(spawnPoint)) {}
};
