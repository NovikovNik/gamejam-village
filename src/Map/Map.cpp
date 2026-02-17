#include "Map.h"
#include <Entities/EntitiesManager.h>
#include <Utils/Singleton.h>
#include <Entities/ETiles.h>
#include <Entities/EPlayer.h>
#include "../Renderer/Camera.h"
#include "Logger/Logger.h"
#include <Entities/EColliders.h>
#include <Entities/EPit.h>
#include <Entities/EBox.h>
#include <Entities/ENpc.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>

class Map : public Singleton<Map> {

public:
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        UnloadCurrentMap();
        currentLevel = filename;

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

        // Load colliders from JSON
        World::EColliders* colliders = entitiesManager.SpawnEntity<World::EColliders>();
        if (jsonData.contains("colliders") && jsonData["colliders"].is_array()) {
            std::vector<World::EColliders::Collider> collidersData;
            for (const auto& colliderJson : jsonData["colliders"]) {
                World::EColliders::Collider collider;
                collider.x = colliderJson["x"].get<float>();
                collider.y = colliderJson["y"].get<float>();
                collider.width = colliderJson["width"].get<float>();
                collider.height = colliderJson["height"].get<float>();
                collidersData.push_back(collider);
            }
            colliders->LoadColliders(collidersData);
            colliders->EnableRender();
        }
        
        // Load pits from JSON
        if (jsonData.contains("pits") && jsonData["pits"].is_array()) {
            for (const auto& pitJson : jsonData["pits"]) {
                const std::string matchBoxName = pitJson["matchBoxName"].get<std::string>();
                World::EPit* pit = entitiesManager.SpawnEntity<World::EPit>(make_nnBoxName(matchBoxName) );
                const float x = pitJson["x"].get<float>();
                const float y = pitJson["y"].get<float>();

                pit->SetPosition(x, y);
            }
        }
        
        // Load boxes from JSON
        if (jsonData.contains("boxes") && jsonData["boxes"].is_array()) {
            for (const auto& boxJson : jsonData["boxes"]) {
                const std::string boxName = boxJson["name"].get<std::string>();
                World::EBox* box = entitiesManager.SpawnEntity<World::EBox>(make_nnBoxName(boxName));
                const float x = boxJson["x"].get<float>();
                const float y = boxJson["y"].get<float>();

                box->SetPosition(x, y);
            }
        }

        // Load npcs from JSON
        if (jsonData.contains("npcs") && jsonData["npcs"].is_array()) {
            for (const auto& npcJson : jsonData["npcs"]) {
                const std::string npcName = npcJson["name"].get<std::string>();
                const float x = npcJson["x"].get<float>();
                const float y = npcJson["y"].get<float>();
                World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
            }
        }
        
        // Load interactables from JSON
        if (jsonData.contains("interactables") && jsonData["interactables"].is_array()) {
            for (const auto& interactableJson : jsonData["interactables"]) {
                const std::string interactableName = interactableJson["name"].get<std::string>();
                const float x = interactableJson["x"].get<float>();
                const float y = interactableJson["y"].get<float>();
                const float width = interactableJson["width"].get<float>();
                const float height = interactableJson["height"].get<float>();
                World::EInteractable* interactable = entitiesManager.SpawnEntity<World::EInteractable>(make_nnInteractId(interactableName));
                interactable->LoadData(make_nnTex(interactableJson["texture"].get<std::string>()), x, y, width, height);
            }
        }

        // Load player from JSON
        if (jsonData.contains("player")) {
            World::EPlayer* player = entitiesManager.SpawnEntity<World::EPlayer>();
            float playerX = jsonData["player"]["x"].get<float>();
            float playerY = jsonData["player"]["y"].get<float>();
            player->LoadData("player"_nnTex, playerX, playerY, 64, 64);
            World::Camera::instance().Follow(player);
            // TODO: Set player position (may need to add SetPosition method to EMovable)
        }

        return true;
    }

    void ReloadMap() {
        LoadMap(currentLevel);
    }

    void UnloadCurrentMap() {
        World::Camera::instance().Unfollow();
        entitiesManager.Clear();
    }

    void Update(float deltaTime) {
        entitiesManager.Update(deltaTime);
        World::Camera::instance().Update(deltaTime);
    }

    void Render(float deltaTime) {
        entitiesManager.Render(deltaTime);
    }

    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer() {
        return entitiesManager.GetEntitiesContainer();
    }

private:
    World::EntitiesManager entitiesManager;
    World::EPlayer* player = nullptr;
    std::string currentLevel;
};

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        return Map::instance().LoadMap(filename);
    }

    void ReloadMap() {
        Map::instance().ReloadMap();
    }

    void UnloadCurrentMap() {
        Map::instance().UnloadCurrentMap();
    }

    void Update(float deltaTime) {
        Map::instance().Update(deltaTime);
    }

    void Render(float deltaTime) {
        Map::instance().Render(deltaTime);
    }

    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer() {
        return Map::instance().GetEntitiesContainer();
    }
};
