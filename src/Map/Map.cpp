#include "Map.h"
#include <Entities/EntitiesManager.h>
#include <Utils/Singleton.h>
#include <Entities/ETiles.h>

class Map : public Singleton<Map> {

public:
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        // Destroys the previous map

        World::ETiles* tiles = entitiesManager.SpawnEntity<World::ETiles>();
        
        std::vector<World::ETiles::Tile> tilesData;
        tilesData.push_back({ 0, 0, 256, 256, "test-texture"_nnTex });
        tilesData.push_back({ 256, 0, 256, 256, "test-texture"_nnTex });
        tilesData.push_back({ 0, 256, 256, 256, "test-texture"_nnTex });

        tiles->LoadTiles(tilesData);
        return true;
    }

    void Update(float deltaTime) {
        entitiesManager.Update(deltaTime);
    }

    void Render(float deltaTime) {
        entitiesManager.Render(deltaTime);
    }

private:
    World::EntitiesManager entitiesManager;
};


namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        return Map::instance().LoadMap(filename);
    }

    void Update(float deltaTime) {
        Map::instance().Update(deltaTime);
    }

    void Render(float deltaTime) {
        Map::instance().Render(deltaTime);
    }
};
