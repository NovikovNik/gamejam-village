#pragma once

#include "Event.h"
#include <string>

class DialogEndedEvent: public Event {
    public:
        std::string characterId;
        std::string dialogId;

        DialogEndedEvent(std::string characterId, std::string dialogId): characterId(characterId), dialogId(dialogId) {}
};