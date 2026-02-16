#include "EPlayer.h"
#include "../Game/GameStates.h"
#include <Map/Map.h>
#include <Entities/EBox.h>

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
        OnMoved();
     }

     return true;
 }

void World::EPlayer::OnMoved() {
    std::vector<World::EBox*> boxes;
    MapManager::GetEntitiesContainer().FindEntities(boxes);
    for (const auto& box : boxes) {
        const auto boxPosition = box->GetPosition();
        const auto playerPosition = GetPosition();
        const auto delta = boxPosition - playerPosition;

        const auto minDistance = 64.f;
        if (abs(delta.y) > abs(delta.x)) {
            if (delta.y > 0 && delta.y < minDistance / 1.1f) { 
                box->Move(0, 1, basicSpeed);
            }
            else if (delta.y < 0 && delta.y > -minDistance / 1.5f) {
                box->Move(0, -1, basicSpeed);
            }
        }
        else {
            if (delta.x > 0 && delta.x < minDistance / 1.5f) {
                box->Move(1, 0, basicSpeed);
            }
            else if (delta.x < 0 && delta.x > -minDistance / 1.5f) {
                box->Move(-1, 0, basicSpeed);
            }
        }
    }
}

void World::EPlayer::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
