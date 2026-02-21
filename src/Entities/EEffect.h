#pragma once

#include "Entity.h"
#include <glm/glm.hpp>
#include <Renderer/Renderer.h>

namespace World {
    class Effect : public Entity {
    public:
        Effect() = default;
        Effect(const Renderer::AnimationHandle& animation);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

    protected:
        Renderer::AnimationHandle animation{};

        bool isFinished = false;
    };
}
