#include "ETiles.h"

bool World::ETiles::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::ETiles::Render(float deltaTime) {
    for (const auto& tile : tiles) {
        Renderer::DrawSprite(tile.texture, tile.x, tile.y, tile.width, tile.height);
    }
}

void World::ETiles::LoadTiles(const std::vector<Tile>& tiles) {
    this->tiles = tiles;
}
