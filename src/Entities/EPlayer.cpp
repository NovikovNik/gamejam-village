#include "EPlayer.h"
#include "../Game/GameStates.h"

bool World::EPlayer::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    glm::vec2 direction(0.0f);

     if (GameStates::instance().w) {
         direction.y -= 1.0f;
     }
     if (GameStates::instance().s) {
         direction.y += 1.0f;
     }
     if (GameStates::instance().a) {
         direction.x -= 1.0f;
     }
     if (GameStates::instance().d) {
         direction.x += 1.0f;
     }

     if (glm::length(direction) > 0.0f) {
         direction = glm::normalize(direction);
         AddImpulse(direction.x * basicSpeed,
                    direction.y * basicSpeed);
     }

     return true;
 }

void World::EPlayer::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
