#include "EntitiesManager.h"
#include <algorithm>

void World::EntitiesManager::Update(float deltaTime) {
    entities.ForEachEntity([&](Entity* entity) {
        if (!entity->Update(deltaTime)) {
            entity->OnDestroy();
        }
    });
    entities.RemoveInvalidEntities();
}

void World::EntitiesManager::Render(float deltaTime) {
    entities.ForEachEntity([&](Entity* entity) {
        if (entity->IsValid()) {
            entity->Render(deltaTime);
        }
    });
}

void World::EntitiesManager::Clear() {
    entities.Clear();
}
