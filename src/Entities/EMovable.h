#pragma once

#include "EMapObject.h"
#include <glm/glm.hpp>
#include <Renderer/Renderer.h>

namespace World {
    class EMovable : public EMapObject {
    public:
        bool Update(float deltaTime) override;

        void AddImpulse(float x, float y);
        glm::vec2 GetVelocity() const { return glm::vec2(velocityX, velocityY); }

    private:
        float velocityX = 0.0f;
        float velocityY = 0.0f;
    };
}
