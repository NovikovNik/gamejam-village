#pragma once

#include "Entity.h"
#include <Renderer/Renderer.h>

namespace World {
    class EMovable : public Entity {
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void LoadData(Renderer::TextureId texture, float width, float height);

        void AddImpulse(float x, float y);

    private:
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float positionX = 0.0f;
        float positionY = 0.0f;

        Renderer::TextureId texture;
        float width = 0.0f;
        float height = 0.0f;
    };
}
