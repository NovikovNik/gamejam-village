#pragma once

#include "Event.h"
#include <string>

class InteractWithEntityEvent: public Event {
    public:
        std::string entityId;

        InteractWithEntityEvent(const std::string& entityId) : entityId(entityId) {}
};