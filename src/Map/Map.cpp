#include "Map.h"
#include <Entities/EntitiesManager.h>
#include <Utils/Singleton.h>
#include <Entities/ETiles.h>
#include <Entities/EPlayer.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <fstream>
#include <sstream>

class Map : public Singleton<Map> {

public:
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        // Destroys the previous map

        // Load JSON map data
        std::string filepath = filename;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        nlohmann::json jsonData = nlohmann::json::parse(buffer.str());

        // Load tiles from JSON
        World::ETiles* tiles = entitiesManager.SpawnEntity<World::ETiles>();
        if (jsonData.contains("tiles") && jsonData["tiles"].is_array()) {
            std::vector<World::ETiles::Tile> tilesData;
            for (const auto& tileJson : jsonData["tiles"]) {
                World::ETiles::Tile tile;
                tile.x = tileJson["x"].get<float>();
                tile.y = tileJson["y"].get<float>();
                tile.width = tileJson["width"].get<float>();
                tile.height = tileJson["height"].get<float>();
                std::string textureName = tileJson["texture"].get<std::string>();
                tile.texture = make_nnTex(textureName);
                tilesData.push_back(tile);
            }
            tiles->LoadTiles(tilesData);
        }

        // Load player from JSON
        if (jsonData.contains("player")) {
            World::EPlayer* player = entitiesManager.SpawnEntity<World::EPlayer>();
            float playerX = jsonData["player"]["x"].get<float>();
            float playerY = jsonData["player"]["y"].get<float>();
            player->LoadData("player"_nnTex, playerX, playerY, 64, 64);
            // TODO: Set player position (may need to add SetPosition method to EMovable)
        }
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
