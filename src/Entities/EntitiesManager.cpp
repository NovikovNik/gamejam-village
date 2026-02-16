#include "EntitiesManager.h"
#include <algorithm>

void World::EntitiesManager::Update(float deltaTime) {
    for (auto& entity : entities) {
        if (!entity->Update(deltaTime)) {
            entity->OnDestroy();
        }
    }

    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<Entity>& entity) {
        return !entity->IsValid();
    }), entities.end());
}

void World::EntitiesManager::Render(float deltaTime) {
    for (auto& entity : entities) {
        if (entity->IsValid()) {
            entity->Render(deltaTime);
        }
    }
}
