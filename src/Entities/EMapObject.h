#pragma once

#include "Entity.h"
#include <Renderer/Renderer.h>

namespace World {
    class EMapObject : public Entity {
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void LoadData(Renderer::TextureId texture, float width, float height);

        float GetWidth() const { return width; }
        float GetHeight() const { return height; }

    private:
        const Renderer::TextureId testTextureId = make_nnTex("test-texture");

    protected:
        Renderer::TextureId texture;
    };
}
