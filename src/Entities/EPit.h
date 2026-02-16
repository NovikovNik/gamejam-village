#pragma once
#include "EMovable.h"

namespace World {
    class EPit : public EMovable {
    public:
        void OnSpawn() override;
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;
    };
}
