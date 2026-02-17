#pragma once

#include "EMapObject.h"
#include <glm/glm.hpp>
#include <Renderer/Renderer.h>

namespace World {
    class EMovable : public EMapObject {
    public:
        bool Update(float deltaTime) override;

        void AddImpulse(float x, float y);

    private:
        float velocityX = 0.0f;
        float velocityY = 0.0f;
    };
}
