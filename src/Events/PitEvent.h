#pragma once

#include "Event.h"
#include <Entities/BoxName.h>

class PitBoxOverlapEvent : public Event {
public:
    BoxName boxName;
    World::Entity::TagName pitTagName;
    int32_t boxPhysicsId;

    PitBoxOverlapEvent(BoxName boxName, World::Entity::TagName pitTagName, int32_t boxPhysicsId) : boxName(std::move(boxName)), pitTagName(std::move(pitTagName)), boxPhysicsId(boxPhysicsId) {}
};
