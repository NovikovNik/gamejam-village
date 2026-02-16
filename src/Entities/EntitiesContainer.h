#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Entity.h"

namespace World {
    class EntitiesContainer {
    public:
        void AddEntity(std::unique_ptr<Entity>&& entity) {
            entities.push_back(std::move(entity));
        }

        void RemoveInvalidEntities() {
            entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<Entity>& e) {
                return !e->IsValid();
            }), entities.end());
        }

        void Clear() {
            entities.clear();
        }

        void ForEachEntity(auto&& callback) const {
            for (auto& entity : entities) {
                callback(entity.get());
            }
        }

        [[nodiscard]] size_t GetEntityCount() const {
            return entities.size();
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

    private:
        std::vector<std::unique_ptr<Entity>> entities;
    };
}