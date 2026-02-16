#pragma once

#include <vector>
#include <memory>
#include "EntitiesContainer.h"

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
                entities.AddEntity(std::move(entity));
                return entityPtr;
            }

            [[nodiscard]] size_t GetEntityCount() const { return entities.GetEntityCount(); }
            [[nodiscard]] const EntitiesContainer& GetEntitiesContainer() const { return entities; }

        private:
            EntitiesContainer entities;
    };
}
