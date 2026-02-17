#pragma once

#include "Entity.h"
#include <glm/glm.hpp>
#include <Renderer/Renderer.h>

namespace World {
    class EMapObject : public Entity {
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void LoadData(Renderer::TextureId texture, float x, float y, float width, float height);

        void SetPosition(float x, float y);
        glm::vec2 GetPosition() const;
        float GetWidth() const { return width; }
        float GetHeight() const { return height; }

    protected:
        float positionX = 0.0f;
        float positionY = 0.0f;

        Renderer::TextureId texture;
        float width = 0.0f;
        float height = 0.0f;
    };
}
