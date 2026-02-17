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
#include <Entities/ESpawners.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

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

        mapData = nlohmann::json::parse(buffer.str());

        // Load tiles from JSON
        World::ETiles* tiles = entitiesManager.SpawnEntity<World::ETiles>();
        if (mapData.contains("tiles") && mapData["tiles"].is_array()) {
            std::vector<World::ETiles::Tile> tilesData;
            for (const auto& tileJson : mapData["tiles"]) {
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
        if (mapData.contains("colliders") && mapData["colliders"].is_array()) {
            std::vector<World::EColliders::Collider> collidersData;
            for (const auto& colliderJson : mapData["colliders"]) {
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
        if (mapData.contains("pits") && mapData["pits"].is_array()) {
            for (const auto& pitJson : mapData["pits"]) {
                const std::string matchBoxName = pitJson["matchBoxName"].get<std::string>();
                World::EPit* pit = entitiesManager.SpawnEntity<World::EPit>(make_nnBoxName(matchBoxName) );
                const float x = pitJson["x"].get<float>();
                const float y = pitJson["y"].get<float>();

                pit->SetPosition(x, y);
            }
        }

        std::vector<World::ESpawners::Spawner> spawners;
        
        // Load boxes from JSON
        if (mapData.contains("boxes") && mapData["boxes"].is_array()) {
            for (const auto& boxJson : mapData["boxes"]) {
                const std::string boxName = boxJson["name"].get<std::string>();
//                World::EBox* box = entitiesManager.SpawnEntity<World::EBox>(make_nnBoxName(boxName));
                const float x = boxJson["x"].get<float>();
                const float y = boxJson["y"].get<float>();

                spawners.push_back({ boxName, "box", x, y });

//                box->SetPosition(x, y);
//                box->SetTagName(std::format("{}.box", boxName));
            }
        }

        // Load npcs from JSON
        if (mapData.contains("npcs") && mapData["npcs"].is_array()) {
            for (const auto& npcJson : mapData["npcs"]) {
                const std::string npcName = npcJson["name"].get<std::string>();
                const float x = npcJson["x"].get<float>();
                const float y = npcJson["y"].get<float>();
                spawners.push_back({ npcName, "villager", x, y });
//                World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
//                npc->SetTagName(std::format("{}.villager", npcName));
            }
        }

        World::ESpawners* spawnersEntity = entitiesManager.SpawnEntity<World::ESpawners>();
        spawnersEntity->LoadSpawners(spawners);
        
        // Load interactables from JSON
        if (mapData.contains("interactables") && mapData["interactables"].is_array()) {
            for (const auto& interactableJson : mapData["interactables"]) {
                const std::string interactableName = interactableJson["name"].get<std::string>();
                const float x = interactableJson["x"].get<float>();
                const float y = interactableJson["y"].get<float>();
                const float width = interactableJson["width"].get<float>();
                const float height = interactableJson["height"].get<float>();
                World::EInteractable* interactable = entitiesManager.SpawnEntity<World::EInteractable>(make_nnInteractId(interactableName));
                interactable->LoadData(make_nnTex(interactableJson["texture"].get<std::string>()), x, y, width, height);
            }
        }

        // Load players from JSON
        if (mapData.contains("players") && mapData["players"].is_array()) {
            for (const auto& playerJson : mapData["players"]) {
                const std::string playerName = playerJson["name"].get<std::string>();
                const float x = playerJson["x"].get<float>();
                const float y = playerJson["y"].get<float>();
                World::EPlayer* player = entitiesManager.SpawnEntity<World::EPlayer>(playerName, x, y);
                World::Camera::instance().Follow(player);
            }
        }
        return true;
    }

    void ReloadMap() {
        LoadMap(currentLevel);
    }

    void UnloadCurrentMap() {
        EventBus::instance().Reset();
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

    World::Entity* SpawnEntity(const std::string& name, const std::string& type) {

        const auto spawners = entitiesManager.GetEntitiesContainer().FindEntity<World::ESpawners>();
        if (spawners) {
            const auto optPosition = spawners->GetSpawnerPosition(name, type);
            if (!optPosition.has_value()) {
                return nullptr;
            }

            const auto position = optPosition.value();
            if (type == "villager") {
                auto npc = entitiesManager.SpawnEntity<World::ENpc>(name, position.x, position.y);
                npc->SetTagName(std::format("{}.{}", name, type));
                return npc;
            }
            if (type == "box") {
                auto box = entitiesManager.SpawnEntity<World::EBox>(make_nnBoxName(name));
                box->SetPosition(position.x, position.y);
                box->SetTagName(std::format("{}.{}", name, type));
                return box;
            }
        }

//        if (type == "villager") {
//            if (mapData.contains("npcs") && mapData["npcs"].is_array()) {
//                for (const auto& npcJson : mapData["npcs"]) {
//                    const std::string npcName = npcJson["name"].get<std::string>();
//                    if (npcName == name) {
//                        const float x = npcJson["x"].get<float>();
//                        const float y = npcJson["y"].get<float>();
//                        World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
//                        npc->SetTagName(std::format("{}.villager", npcName));
//                        return npc;
//                    }
//                }
//            }
//        }
        return nullptr;
    }

    std::string GetCurrentMapName() const {
        return std::filesystem::path(currentLevel).filename().stem().string();
    }

private:
    World::EntitiesManager entitiesManager;
    World::EPlayer* player = nullptr;
    std::string currentLevel;

    nlohmann::json mapData;
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

    const World::EntitiesContainer& GetEntitiesContainer() {
        return Map::instance().GetEntitiesContainer();
    }

    World::Entity* SpawnEntity(const std::string& name, const std::string& type) {
        return Map::instance().SpawnEntity(name, type);
    }

    std::string GetCurrentMapName() {
        return Map::instance().GetCurrentMapName();
    }
};
