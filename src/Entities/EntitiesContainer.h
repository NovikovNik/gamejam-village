#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Entity.h"

namespace World {
    class EntitiesContainer {
    public:
        void AddEntity(std::unique_ptr<Entity>&& entity);
        void RemoveInvalidEntities();
        void Clear();

        [[nodiscard]] size_t GetEntityCount() const;

        [[nodiscard]] bool Contains(const Entity* ptr) const {
            for (const auto& entity : entities) {
                if (entity.get() == ptr) return true;
            }
            return false;
        }

        void ForEachEntity(auto&& callback) const {
            for (auto& entity : entities) {
                callback(entity.get());
            }
        }

        template <typename T>
        [[nodiscard]] T* FindEntity() const {
            for (auto& entity : entities) {
                if (!entity) {
                    continue;
                }
                if (!entity->IsValid()) {
                    continue;
                }
                auto entityPtr = dynamic_cast<T*>(entity.get());
                if (entityPtr != nullptr) {
                    return entityPtr;
                }
            }
            return nullptr;
        }
        
        [[nodiscard]] Entity* FindEntity(const Entity::TagName& tagName) const {
            for (auto& entity : entities) {
                if (!entity) {
                    continue;
                }
                if (!entity->IsValid()) {
                    continue;
                }
                if (entity->GetTagName() == tagName) {
                    return entity.get();
                }
            }
            return nullptr;
        }

        template <typename T>
        void FindEntities(std::vector<T*>& entitiesFound) const {
            for (auto& entity : entities) {
                if (!entity) {
                    continue;
                }
                if (!entity->IsValid()) {
                    continue;
                }
                auto entityPtr = dynamic_cast<T*>(entity.get());
                if (entityPtr != nullptr) {
                    entitiesFound.push_back(entityPtr);
                }
            }
        }

    private:
        std::vector<std::unique_ptr<Entity>> entities;
    };
}