#pragma once

#include <vector>
#include <memory>
#include "EntitiesContainer.h"

namespace World {
    class EntitiesManager {
        public:
            void Update(float deltaTime);
            void Render(float deltaTime);
            void Clear();

            template <typename T, typename... Args>
            T* SpawnEntity(float x, float y, float w, float h, Args&&... args) {
                auto entity = std::make_unique<T>(std::forward<Args>(args)...);
                entity->OnSpawn(x, y, w, h);
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
