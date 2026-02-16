#pragma once

#include "EMovable.h"
#include <glm/glm.hpp>

namespace World {
    class EPlayer : public EMovable {
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

    private:
        // Pixels per frame

        glm::vec2 direction; // for normalization kinda
        float basicSpeed = 100.0f;
    };
}
