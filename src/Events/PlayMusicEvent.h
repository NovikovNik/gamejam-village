#pragma once

#include "Event.h"
#include <string>

/// Emit this event to start looping background music via AudioSystem.
class PlayMusicEvent : public Event {
public:
    explicit PlayMusicEvent(std::string musicId)
        : musicId(std::move(musicId)) {}

    std::string musicId;
};
