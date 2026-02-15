#pragma once

#include "../ECS/ECS.h"
#include "Event.h"

class CollisionEvent: public Event {
    public:
        Entity entityA;
        Entity entityB;

        CollisionEvent(Entity a, Entity b): entityA(a), entityB(b) {}
};