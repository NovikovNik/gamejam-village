#pragma once

#include "Event.h"
#include <string>

class ForceDialogStartEvent: public Event {
    public:
        std::string characterId;
        std::string dialogId;

        ForceDialogStartEvent(std::string characterId, std::string dialogId): characterId(characterId), dialogId(dialogId) {}
};