#pragma once

#include "EMovable.h"
#include <glm/glm.hpp>

namespace World {
    class EPlayer : public EMovable {
    public:
        EPlayer(const std::string& name, float x, float y);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void OnMoved(float deltaTime);

        [[nodiscard]] class EInteractable* TryInteract() const;

    private:
        // Pixels per frame

        glm::vec2 direction; // for normalization kinda
        float basicSpeed = 125.0f;
        std::string name;
    };
}
