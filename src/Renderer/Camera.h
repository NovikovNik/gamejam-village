#pragma once

#include "../Utils/Singleton.h"
#include "../Entities/EMovable.h"
#include <glm/vec2.hpp>

namespace World {
    class Camera: public Singleton<Camera> {
        public:
            void Follow(const EMovable* target);
            void Unfollow();
            void Update(float dt);

            glm::vec2 GetPosition() const;

        private:
            const EMovable* followTarget = nullptr;
            float positionX;
            float positionY;
            float smoothSpeed = 6.0f;
    };
}
