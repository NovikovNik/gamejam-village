#include "EPlayer.h"
#include "../Game/GameStates.h"

bool World::EPlayer::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    if (GameStates::instance().w) {
        AddImpulse(0, -basicSpeed);
    }
    if (GameStates::instance().s) {
        AddImpulse(0, basicSpeed);
    }
    if (GameStates::instance().a) {
        AddImpulse(-basicSpeed, 0);
    }
    if (GameStates::instance().d) {
        AddImpulse(basicSpeed, 0);
    }

    return true;
}

void World::EPlayer::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
