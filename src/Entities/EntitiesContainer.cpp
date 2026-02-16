#include "EntitiesContainer.h"

void World::EntitiesContainer::AddEntity(std::unique_ptr<World::Entity>&& entity) {
    entities.push_back(std::move(entity));
}

void World::EntitiesContainer::RemoveInvalidEntities() {
    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<World::Entity>& e) {
        return !e->IsValid();
    }), entities.end());
}

void World::EntitiesContainer::Clear() {
    entities.clear();
}

[[nodiscard]] size_t World::EntitiesContainer::GetEntityCount() const {
    return entities.size();
}
