#pragma once

#include <vector>
#include <memory>
#include "Entity.h"

namespace World {
    class EntitiesManager {
        public:
            void Update(float deltaTime);
            void Render(float deltaTime);

            template <typename T, typename... Args>
            T* SpawnEntity(Args&&... args) {
                auto entity = std::make_unique<T>(std::forward<Args>(args)...);
                entity->OnSpawn();
                auto* entityPtr = entity.get();
                entities.push_back(std::move(entity));
                return entityPtr;
            }

            [[nodiscard]] size_t GetEntityCount() const { return entities.size(); }

        private:
            std::vector<std::unique_ptr<Entity>> entities;
    };
}
