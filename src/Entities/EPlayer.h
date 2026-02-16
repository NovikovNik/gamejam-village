#pragma once

#include "EMovable.h"

namespace World {
    class EPlayer : public EMovable {
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

    private:
        // Pixels per frame
        float basicSpeed = 50.0f;
    };
}
