#pragma once

#include "Entity.h"
#include <vector>

namespace World {
    class EColliders : public Entity {
    public:
        struct Collider {
            float x;
            float y;
            float width;
            float height;
        };
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;
        void LoadColliders(const std::vector<Collider>& colliders);

        [[nodiscard]] const std::vector<Collider>& GetColliders() const { return colliders; }

        void EnableRender() { bRender = true; }
        void DisableRender() { bRender = false; }

    private:
        std::vector<Collider> colliders;
        bool bRender = false;
    };
}
