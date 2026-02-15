#pragma once

#include <bitset>
#include <deque>
#include <memory>
#include <string>
#include "../Logger/Logger.h"
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <unordered_map>
#include <set>
#include <stdexcept>

const unsigned int MAX_COMPONENTS = 32;
// Signature is a bitset of 1 and 0, to keet track of which components an entity has //
// And also helps keep track of which system are interested in an entity
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
    protected:
        static int nextId;
};

template <typename T>
class Component: public IComponent {
    // Get unique id for each component type
    public:
        static int GetId() {
            static auto id = nextId++;
            return id;
        }
};

class Entity {
    private:
        int id;

    public:
        Entity(int id): id(id) {}
        inline int GetId() const { return id; }

        bool operator==(const Entity& other) const;
        bool operator!=(const Entity& other) const;
        bool operator<(const Entity& other) const;
        bool operator>(const Entity& other) const;

        class Registry* registry;

        template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
        template <typename TComponent> void RemoveComponent();
        template <typename TComponent> bool HasComponent() const;
        template <typename TComponent> TComponent& GetComponent() const;
        void Kill();

        void Tag(const std::string& tag);
        void RemoveTag();
        bool HasTag(const std::string& tag);
        void Group(const std::string& group);
        void RemoveGroup();
        bool BelongsToGroup(const std::string& group);
};

// System class //
// Processing entities that contain a specific signature //
class System {
    private:
        Signature componentSignature;
        std::vector<Entity> entities;

    public:
        System() = default;
        ~System() = default;

        void AddEntityToSystem(Entity entity);
        void RemoveEntityFromSystem(Entity entity);
        std::vector<Entity> GetSystemEntities() const;
        const Signature& GetComponentSignature() const;

        // Defines the component type that entities must have to be considered by this system
        template <typename TComponent> void RequireComponent();
};

template <typename TComponent>
void System::RequireComponent() {
    const auto componentId = Component<TComponent>::GetId();
    componentSignature.set(componentId);
}

class IPool {
    public:
        virtual ~IPool() = default;
        virtual void RemoveEntityFromPool(int entityId) = 0;
};

// Pool class is just a vector of objects of type T
template <typename T>
class Pool: public IPool {
    private:
        std::vector<T> data;
        int size;

        std::unordered_map<int, int> entityIdToIndex;
        std::unordered_map<int, int> indexToEntityId;

    public:
        Pool(int capacity = 100) {
            size = 0;
            data.resize(capacity);
        }
        virtual ~Pool() = default;

    public:
        bool IsEmpty() const {
            return size == 0;
        }

        int GetSize() const {
            return size;
        }

        void Resize(int size) {
            data.resize(size);
        }

        void Clear() {
            data.clear();
            size = 0;
        }

        void Add(const T& object) {
            data.push_back(object);
        }

        void Set(int entityId, const T& object) {
            if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
                // If the element exists — replace component
                int index = entityIdToIndex[entityId];
                data[index] = object;
            } else {
                int index = size;
                entityIdToIndex.emplace(entityId, index);
                indexToEntityId.emplace(index, entityId);
                if (index >= data.capacity()) {
                    // Do this if necesery
                    data.resize(size * 2);
                }
                data[index] = object;
                size++;
            }
        }

        void Remove(int entityId) {
            int indexOfRemoved = entityIdToIndex[entityId];
            int indexOfLast = size - 1;
            data[indexOfRemoved] = data[indexOfLast];

            int entityIdOfLastElement = indexToEntityId[indexOfLast];
            entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
            indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

            entityIdToIndex.erase(entityId);
            indexToEntityId.erase(indexOfLast);

            size--;
        }

        void RemoveEntityFromPool(int entityId) override {
            if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
                Remove(entityId);
            }
        }

        T& Get(int entityId) {
            int index = entityIdToIndex[entityId];
            return static_cast<T&>(data[index]);
        }

        T& operator[](unsigned int index) {
            return data[index];
        }
};

// Registry manages the creation and destruction of entities and components, add systems and components
class Registry {
    private:
        int numEntities = 0;
        std::deque<int> freeIds;
        std::set<Entity> entitiesToBeAdded ;
        std::set<Entity> entitiesToBeKilled;

        std::unordered_map<std::string, Entity> entityPerTag;
        std::unordered_map<int, std::string> tagPerEntity;

        std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
        std::unordered_map<int, std::string> groupPerEntity;
        // Vector index = component type id
        // Pool index = entity id
        std::vector<std::shared_ptr<IPool>> componentPools;

        // Vector of component signatures per entity which component is turned "on" for
        // [Vector index = entity id]
        std::vector<Signature> entityComponentSignatures;
        std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

    public:
        Registry() = default;
        ~Registry() = default;
    public:
        void Update();
        // Create a new entity
        Entity CreateEntity();

        template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
        template <typename TComponent> void RemoveComponent(Entity entity);
        template <typename TComponent> bool HasComponent(Entity entity);
        template <typename TComponent> TComponent& GetComponent(Entity entity);
        //
        template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
        template <typename TSystem> void RemoveSystem();
        template <typename TSystem> bool HasSystem() const;
        template <typename TSystem> TSystem& GetSystem() const;

        // Checks the component signature of an entity and add the entity to all the systems that match
        void AddEntityToSystems(Entity entity);
        void RemoveEntityFromSystems(Entity entity);
        void KillEntity(Entity entity);

        void TagEntity(Entity entity, const std::string& tag);
        bool EntityHasTag(Entity entity, const std::string& tag) const;
        std::optional<Entity> GetEntityByTag(const std::string& tag) const;
        void RemoveEntityTag(Entity entity);

        void GroupEntity(Entity entity, const std::string& group);
        bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
        std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
        void RemoveEntityGroup(Entity entity);
};

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= componentPools.size()) {
        componentPools.resize(componentId + 1, nullptr);
    }

    if (componentPools[componentId] == nullptr) {
        std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
        componentPools[componentId] = newComponentPool;
    }
    // Get the pool of component values for that component type
    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

    TComponent newComponent(std::forward<TArgs>(args)...);
    componentPool->Set(entityId, newComponent);

    entityComponentSignatures[entityId].set(componentId);

    Logger::Log("Component id = " + std::to_string(componentId) + " added to entity id = " + std::to_string(entityId));
    Logger::Debug("COMPONENT_ID: " + std::to_string(componentId) + " POOL SIZE: " + std::to_string(componentPool->GetSize()));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    entityComponentSignatures[entityId].set(componentId, false);

    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    componentPool->Remove(entityId);
    Logger::Log("Component id = " + std::to_string(componentId) + " removed from entity id = " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    return std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId])->Get(entityId);
}

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
    std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem() {
    const auto systemId = std::type_index(typeid(TSystem));
    systems.erase(systemId);
}

template <typename TSystem>
bool Registry::HasSystem() const {
    return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    if (system != systems.end()) {
        return *std::static_pointer_cast<TSystem>(system->second);
    }
    throw std::runtime_error("System not found");
}

template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
    registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
    registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
    return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
    return registry->GetComponent<TComponent>(*this);
}
