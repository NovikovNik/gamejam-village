#pragma once

#include "Event.h"
#include <string>

/// Emit this event to play a one-shot sound effect via AudioSystem.
class PlaySoundEvent : public Event {
public:
    explicit PlaySoundEvent(std::string soundId, float volume = 1.0f)
        : soundId(std::move(soundId)), volume(volume) {}

    std::string soundId;
    float       volume;
};
