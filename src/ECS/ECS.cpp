#include "ECS.h"
#include "../Logger/Logger.h"
#include <optional>
#include <string>

int IComponent::nextId = 0;

void System::AddEntityToSystem(Entity entity) {
    entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
    entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
        return other == entity;
    }), entities.end());
}

bool Entity::operator==(const Entity& other) const {
    return id == other.GetId();
}

bool Entity::operator!=(const Entity& other) const {
    return id != other.GetId();
}

bool Entity::operator<(const Entity& other) const {
    return id < other.GetId();
}

bool Entity::operator>(const Entity& other) const {
    return id > other.GetId();
}

Entity Registry::CreateEntity() {
    int entityId;
    if (freeIds.empty()) {
        entityId = numEntities++;
    } else {
        entityId = freeIds.front();
        freeIds.pop_front();
        Logger::Log("Entity id " + std::to_string(entityId) + " reused");
    }
    Entity entity(entityId);
    entity.registry = this;
    entitiesToBeAdded.insert(entity);
    if (entityId >= entityComponentSignatures.size()) {
        entityComponentSignatures.resize(entityId + 1);
    }
    Logger::Log("Entity created with id: " + std::to_string(entityId));
    return entity;
}

void Entity::Kill() {
    registry->KillEntity(*this);
}

void Entity::Tag(const std::string& tag) {
    registry->TagEntity(*this, tag);
}

void Entity::RemoveTag() {
    registry->RemoveEntityTag(*this);
}

bool Entity::HasTag(const std::string& tag) {
    return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) {
    registry->GroupEntity(*this, group);
}

void Entity::RemoveGroup() {
    registry->RemoveEntityGroup(*this);
}

bool Entity::BelongsToGroup(const std::string& group) {
    return registry->EntityBelongsToGroup(*this, group);
}

void Registry::KillEntity(Entity entity) {
    entitiesToBeKilled.insert(entity);
}

void Registry::Update() {
    for (auto entity: entitiesToBeAdded) {
        AddEntityToSystems(entity);
    }
    for (auto entity: entitiesToBeKilled) {
        int entityId = entity.GetId();
        RemoveEntityFromSystems(entity);
        entityComponentSignatures[entityId].reset();

        for (auto pool: componentPools) {
            if (pool) {
                pool->RemoveEntityFromPool(entityId);
            }
        }

        freeIds.push_back(entityId);

        RemoveEntityTag(entity);
        RemoveEntityGroup(entity);
    }
    entitiesToBeAdded.clear();
    entitiesToBeKilled.clear();
}

std::vector<Entity> System::GetSystemEntities() const {
    return entities;
}

const Signature& System::GetComponentSignature() const {
    return componentSignature;
}

void Registry::AddEntityToSystems(Entity entity) {
    const auto entityId = entity.GetId();

    const auto& entityComponentSignature = entityComponentSignatures[entityId];
    for (auto& [systemId, system]: systems) {
        const auto& systemComponentSignature = system->GetComponentSignature();
        bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;

        if (isInterested) {
            system->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity) {
    const auto entityId = entity.GetId();
    const auto& entityComponentSignature = entityComponentSignatures[entityId];
    for (auto& [systemId, system]: systems) {
        const auto& systemComponentSignature = system->GetComponentSignature();
        bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;
        if (isInterested) {
            system->RemoveEntityFromSystem(entity);
        }
    }
}

void Registry::TagEntity(Entity entity, const std::string& tag) {
    entityPerTag.emplace(tag, entity);
    tagPerEntity.emplace(entity.GetId(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const {
    auto entityTag = tagPerEntity.find(entity.GetId());
    if (entityTag != tagPerEntity.end()) {
        return entityPerTag.find(tag)->second == entity;
    }
    return false;
}
std::optional<Entity> Registry::GetEntityByTag(const std::string& tag) const {
    if (auto it = entityPerTag.find(tag); it != entityPerTag.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Registry::RemoveEntityTag(Entity entity) {
    auto entityTag = tagPerEntity.find(entity.GetId());
    if (entityTag != tagPerEntity.end()) {
        auto tag = entityTag->second;
        entityPerTag.erase(tag);
        tagPerEntity.erase(entityTag);
    }
}

void Registry::GroupEntity(Entity entity, const std::string& group) {
    entitiesPerGroup.emplace(group, std::set<Entity>());
    entitiesPerGroup[group].emplace(entity);

    groupPerEntity.emplace(entity.GetId(), group);
}
bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const {
    if (groupPerEntity.find(entity.GetId()) == groupPerEntity.end()) {
        return false;
    }
    return groupPerEntity.find(entity.GetId())->second == group;
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group) const {
    auto groupEntities = entitiesPerGroup.at(group);
    return std::vector<Entity>(groupEntities.begin(), groupEntities.end());
}

void Registry::RemoveEntityGroup(Entity entity) {
    auto groupedEntities = groupPerEntity.find(entity.GetId());
    if (groupedEntities != groupPerEntity.end()) {
        auto group = entitiesPerGroup.find(groupedEntities->second);
        if (group != entitiesPerGroup.end()) {
            auto entityInGroup = group->second.find(entity);
            if(entityInGroup != group->second.end()) {
                group->second.erase(entityInGroup);
            }
        }
        groupPerEntity.erase(groupedEntities);
    }
}
