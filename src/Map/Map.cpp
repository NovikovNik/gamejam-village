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
#include <Entities/EInteractableObject.h>
#include <Entities/ESpawners.h>
#include <Entities/ETriggerLocation.h>
#include <Events/WindowFocusedEvent.h>
#include <Events/ClearWorldStateEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <FileSystem/FileSystem.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Gameplay/WorldState.h>
#include <EventBus/EventBus.h>
#include <Events/EntitiesEvent.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <filesystem>
#include <format>

class Map : public Singleton<Map> {

public:
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        // Load JSON map data
        std::string filepath = filename;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Err(std::format("Failed to load map: {}", filepath));
            return false;
        }

        UnloadCurrentMap();
        currentLevel = filename;

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

        std::vector<World::ESpawners::Spawner> spawners;
        
        // Load pits from JSON
        if (mapData.contains("pits") && mapData["pits"].is_array()) {
            for (const auto& pitJson : mapData["pits"]) {
                const std::string matchBoxName = pitJson["matchBoxName"].get<std::string>();
                //World::EPit* pit = entitiesManager.SpawnEntity<World::EPit>(make_nnBoxName(matchBoxName) );
                const float x = pitJson["x"].get<float>();
                const float y = pitJson["y"].get<float>();

                //pit->SetPosition(x, y);
                //pit->SetTagName({ matchBoxName, "pit" });

                spawners.push_back({ matchBoxName, "pit", x, y, true });
            }
        }
        
        // Load boxes from JSON
        if (mapData.contains("boxes") && mapData["boxes"].is_array()) {
            for (const auto& boxJson : mapData["boxes"]) {
                const std::string boxName = boxJson["name"].get<std::string>();
//                World::EBox* box = entitiesManager.SpawnEntity<World::EBox>(make_nnBoxName(boxName));
                const float x = boxJson["x"].get<float>();
                const float y = boxJson["y"].get<float>();
                bool shouldSpawnInstantly = false;
                if (boxJson.contains("spawnOnStart")) {
                    shouldSpawnInstantly = boxJson["spawnOnStart"].get<bool>();
                } else {
                    Logger::Warn(std::format("Box {} does not have spawnOnStart property, defaulting to true", boxName));
                    shouldSpawnInstantly = true;
                }
                spawners.push_back({ boxName, "box", x, y, shouldSpawnInstantly });
            }
        }

        // Load npcs from JSON
        if (mapData.contains("npcs") && mapData["npcs"].is_array()) {
            for (const auto& npcJson : mapData["npcs"]) {
                const std::string npcName = npcJson["name"].get<std::string>();
                const float x = npcJson["x"].get<float>();
                const float y = npcJson["y"].get<float>();
                bool shouldSpawnInstantly = false;
                if (npcJson.contains("spawnOnStart")) {
                    shouldSpawnInstantly = npcJson["spawnOnStart"].get<bool>();
                } else {
                    Logger::Warn(std::format("NPC {} does not have spawnOnStart property, defaulting to true", npcName));
                    shouldSpawnInstantly = true;
                }
                spawners.push_back({ npcName, "villager", x, y, shouldSpawnInstantly });
//                World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
//                npc->SetTagName(std::format("{}.villager", npcName));
            }
        }

        // Load interactible_objects into spawners before LoadSpawners (must be in spawners for SeedInstantSpawn + SpawnEntity)
        if (mapData.contains("interactible_objects") && mapData["interactible_objects"].is_array()) {
            for (const auto& interactableObjectJson : mapData["interactible_objects"]) {
                const std::string interactableObjectName = interactableObjectJson["name"].get<std::string>();
                const float x = interactableObjectJson["x"].get<float>();
                const float y = interactableObjectJson["y"].get<float>();
                bool shouldSpawnInstantly = false;
                if (interactableObjectJson.contains("spawnOnStart")) {
                    shouldSpawnInstantly = interactableObjectJson["spawnOnStart"].get<bool>();
                } else {
                    Logger::Warn(std::format("InteractibleObject {} does not have spawnOnStart property, defaulting to true", interactableObjectName));
                    shouldSpawnInstantly = true;
                }
                spawners.push_back({ interactableObjectName, "object", x, y, shouldSpawnInstantly });
            }
        }

        World::ESpawners* spawnersEntity = entitiesManager.SpawnEntity<World::ESpawners>();
        spawnersEntity->LoadSpawners(spawners);
        
        // Load interactables from JSON (direct spawn, not via spawners)
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

        // Load location triggers from JSON
        if (mapData.contains("levelChangeTriggers") && mapData["levelChangeTriggers"].is_array()) {
            for (const auto& levelChangeTriggerJson : mapData["levelChangeTriggers"]) {
                const std::string locationName = levelChangeTriggerJson["location"].get<std::string>();
                const float x = levelChangeTriggerJson["x"].get<float>();
                const float y = levelChangeTriggerJson["y"].get<float>();
                const float width = levelChangeTriggerJson["width"].get<float>();
                const float height = levelChangeTriggerJson["height"].get<float>();
                World::ETriggerLocation* triggerLocation = entitiesManager.SpawnEntity<World::ETriggerLocation>(locationName);
                // Возможно триггеру не нужна будет текстура в конце концов
                // Хотя это может быть, например, обьект телепорт. Почему бы и нет
                triggerLocation->LoadData(make_nnTex(levelChangeTriggerJson["texture"].get<std::string>()), x, y, width, height);
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
                World::Camera::instance().SetPosition(x, y); // Нужно чтобы не было небольшого смещения камеры при старте уровня
            }
        }

        // Тут первый раз создаем все Entity на локации при старте игры (в файлавой директории)
        const std::string locationName = GetCurrentMapName();
        RevealLocationInFilesystem(locationName);
        if (!IsLocationProcessed(locationName)) {
            SeedInstantSpawnEntitiesInFilesystem();
            MarkLocationProcessed(locationName);
        }
        const auto& objects = objectsLocations[locationName];
        for (const auto& object : objects) {
            SpawnEntity(object.name, object.type, true);
        }

        EventBus::instance().EmitEvent<ClearWorldStateEvent>();
        EventBus::instance().EmitEvent<WindowFocusedEvent>();

        // HACK: Чтобы перезагрузить состояние мира после загрузки локации
        return true;
    }

    [[nodiscard]] bool LoadLastLoadedLevel() {
        return LoadMap(ProgressSystemManager::Player().lastLevel);
    }

    void ReloadMap() {
        LoadMap(currentLevel);
    }

    void UnloadCurrentMap() {
        // EventsQueue::instance().Clear();
        // EventBus::instance().Reset();
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

    World::Entity* SpawnEntity(const std::string& name, const std::string& type, bool silently = false) {
        if (const World::Entity* foundEntity = entitiesManager.GetEntitiesContainer().FindEntity({ name, type }); foundEntity != nullptr) {
            return nullptr;
        }

        // Обработка специфичных файлов, которые на самом деле даже некуда спавнить
        if (name == "kick-my" && type == "ass") {
            Logger::Log("[Map] Meme file detected. Skipping spawn but sending event.");
            EventBus::instance().EmitEvent<EntityCreatedEvent>(name, type);
            return nullptr;
        }

        const auto spawners = entitiesManager.GetEntitiesContainer().FindEntity<World::ESpawners>();
        if (spawners) {
            const auto optPosition = spawners->GetSpawnerPosition(name, type);
            if (!optPosition.has_value()) {
                return nullptr;
            }

            if (silently) {
                EventBus::instance().EmitEvent<EntityCreatedEvent>(name, type);
            }

            objectsLocations[GetCurrentMapName()].insert({ { name, type } });
            const auto position = optPosition.value();
            if (type == "villager") {
                auto npc = entitiesManager.SpawnEntity<World::ENpc>(name, position.x, position.y);
                npc->SetTagName({ name, type });
                Logger::Log(std::format("[Map] Spawned NPC: {} of type {}", name, type));
                return npc;
            }
            if (type == "object") {
                auto obj = entitiesManager.SpawnEntity<World::EInteractableObject>(name, position.x, position.y);
                obj->SetTagName({ name, type });
                Logger::Log(std::format("[Map] Spawned interactable object: {} of type {}", name, type));
                return obj;
            }
            if (type == "box") {
                auto box = entitiesManager.SpawnEntity<World::EBox>(make_nnBoxName(name));
                box->SetPosition(position.x, position.y);
                box->SetTagName({ name, type });
                return box;
            }
            if (type == "pit") {
                auto pit = entitiesManager.SpawnEntity<World::EPit>(make_nnBoxName(name));
                pit->SetPosition(position.x, position.y);
                pit->SetTagName({ name, type });
                return pit;
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

    void DestroyEntity(const std::string& name, const std::string& type) {
        if (!type.empty() && !name.empty()) {
            auto entity = entitiesManager.GetEntitiesContainer().FindEntity({ name, type });
            if (entity) {
                EventBus::instance().EmitEvent<EntityDestroyedEvent>(name, type);
                entity->Destroy();
                WorldState::RemoveFromWorldState({ name, type });
            }
        }
        objectsLocations[GetCurrentMapName()].erase({ name, type });
    }

    void OpenCurrentLocationInExplorer() {
        const auto locationName = GetCurrentMapName();
        const auto fullPath = std::filesystem::path("village") / locationName;
        FileSystemManager::OpenSystemExplorer(fullPath.string());
    }

    void SeedInstantSpawnEntitiesInFilesystem() {
        const auto* spawnersEntity = entitiesManager.GetEntitiesContainer().FindEntity<World::ESpawners>();
        if (!spawnersEntity) {
            return;
        }

        const auto locationName = GetCurrentMapName();
        const auto instantSpawns = spawnersEntity->GetInstantSpawnEntities();
        if (instantSpawns.empty()) {
            return;
        }
        
        for (const auto& [name, type] : instantSpawns) {
            objectsLocations[locationName].insert({ { name, type } });\
        }
        Logger::Log(std::format("Seeded {} instant-spawn entities in village/{}", instantSpawns.size(), locationName));
    }

    std::string GetCurrentMapName() const {
        return std::filesystem::path(currentLevel).filename().stem().string();
    }

    // Проверка была ли локация уже посещена или это первый запуск игры
    [[nodiscard]] bool IsLocationProcessed(const std::string& locationId) const {
        return processedLocations.contains(locationId);
    }

    // При первом заходе на локацию (aka новая локация или первый старт игры)
    void MarkLocationProcessed(const std::string& locationId) {
        processedLocations.insert(locationId);
    }

    void RevealLocationInFilesystem(const std::string& locationName) {
        const auto villageDir = std::filesystem::path("village") / locationName;
        FileSystemManager::CreateDirectory(villageDir.string());
    }

private:
    World::EntitiesManager entitiesManager;
    World::EPlayer* player = nullptr;
    std::string currentLevel;

    nlohmann::json mapData;

    // Структура для обозначения, что локация посечена в первый раз
    // При первом заходе на неё нужно создать все Entity с spawnOnStart=true

    std::set<std::string> processedLocations;
    std::map<std::string, std::set<World::Entity::TagName>> objectsLocations;
};

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename) {
        return Map::instance().LoadMap(filename);
    }

    [[nodiscard]] bool LoadLastLoadedLevel() {
        return Map::instance().LoadLastLoadedLevel();
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

    void DestroyEntity(const std::string& name, const std::string& type) {
        Map::instance().DestroyEntity(name, type);
    }

    std::string GetCurrentMapName() {
        return Map::instance().GetCurrentMapName();
    }

    void OpenCurrentLocationInExplorer() {
        Map::instance().OpenCurrentLocationInExplorer();
    }

    void SeedInstantSpawnEntitiesInFilesystem() {
        Map::instance().SeedInstantSpawnEntitiesInFilesystem();
    }

};
