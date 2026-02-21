#include "EntitiesContainer.h"

World::EntitiesContainer::EntitiesContainer()
{
    // To avoid crashes on resize 
    entities.reserve(100);
}

void World::EntitiesContainer::AddEntity(std::unique_ptr<World::Entity>&& entity) {
    entities.push_back(std::move(entity));
}

void World::EntitiesContainer::RemoveInvalidEntities() {
    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<World::Entity>& e) {
        return !e || !e->IsValid();
    }), entities.end());
}

void World::EntitiesContainer::Clear() {
    for (auto& entity : entities) {
        if (entity) {
            entity->OnDestroy();
        }
    }
    entities.clear();
}

[[nodiscard]] size_t World::EntitiesContainer::GetEntityCount() const {
    return entities.size();
}
