#pragma once

#include "Event.h"
#include <Entities/BoxName.h>

class PitBoxOverlapEvent : public Event {
public:
    BoxName boxName;
    World::Entity::TagName pitTagName;

    PitBoxOverlapEvent(BoxName boxName, World::Entity::TagName pitTagName) : boxName(std::move(boxName)), pitTagName(std::move(pitTagName)) {}
};
