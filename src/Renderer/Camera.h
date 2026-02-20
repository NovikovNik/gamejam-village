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
            
            void ResetPosition();
            void SetPosition(float x, float y);
            void SetScaleFactor(float scaleFactor);
            inline float GetScaleFactor() const { return scaleFactor; };
            glm::vec2 GetPosition() const;

        private:
            const EMovable* followTarget = nullptr;
            float positionX;
            float positionY;
            float scaleFactor = 1.7;
            float smoothSpeed = 6.0f;
            float lookAheadTime = 0.2f;   // секунды движения, на которые камера смещается в сторону хода
            float lookAheadMax = 15.0f;    // макс. смещение в мировых единицах
    };
}
