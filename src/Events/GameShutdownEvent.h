#pragma once

#include "Event.h"
#include <string>

class GameShutdownEvent: public Event {
    public:
        GameShutdownEvent() {}
};