#pragma once
#include <Renderer/Renderer.h>
#include <string>
#include <span>
#include <vector>
#include "Entity.h"

namespace World {
    class ETiles : public Entity {
    public:
        struct Tile {
            float x;
            float y;
            float width;
            float height;
            Renderer::TextureId texture;
            bool tiled;
        };
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void LoadTiles(const std::vector<Tile>& tiles);

    private:
        std::vector<Tile> tiles;
    };
}
