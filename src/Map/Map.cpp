#include "Map.h"
#include <Entities/EntitiesManager.h>
#include <Utils/Singleton.h>
#include <Entities/ETiles.h>
#include <Entities/EPlayer.h>
#include "../Renderer/Camera.h"
#include "Logger/Logger.h"
#include <Entities/EColliders.h>
#include <Physics/PhysicsEngine.h>
#include <Entities/EPit.h>
#include <Entities/EBox.h>
#include <Entities/ENpc.h>
#include <Entities/EInteractableObject.h>
#include <Entities/ESpawners.h>
#include <Entities/ETriggerLocation.h>
#include <Entities/EEffect.h>
#include <Events/WindowFocusedEvent.h>
#include <Events/ClearWorldStateEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/LocationChangedEvent.h>
#include <Events/WorldStateEvents.h>
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
#include <map>
#include <filesystem>
#include <format>

class Map : public Singleton<Map> {

public:

    void Initialize() {
        onWorldStateUpdated = EventBus::instance().SubscribeToEvent<WorldStateUpdatedEvent>(this, &Map::OnWorldStateUpdated);
    }

    void Destroy() {
        onWorldStateUpdated.Destroy();
        entitiesManager.Clear();
    }

    void Sync() {
        const auto& state = WorldState::GetCurrentState();

        const auto locationName = GetCurrentMapName();
        std::vector<std::pair<std::string, std::string>> entitiesToDestroy;
        entitiesManager.GetEntitiesContainer().ForEachEntity([&](const World::Entity* entity) {
            const auto name = entity->GetTagName().name;
            const auto type = entity->GetTagName().type;
            const auto key = std::format("{}.{}", name, type);
            if (!state.registeredEntities.contains(key)) {
                return;
            }
            if (state.registeredEntities.at(key).contains(locationName)) {
                return;
            }
            entitiesToDestroy.push_back({ name, type });
        });

        for (const auto& [name, type] : entitiesToDestroy) {
            Renderer::AnimationHandle animation = Renderer::AnimationHandle{
                .numOfFrames = 16,
                .maxElementsPerRow = 4,
                .frameSize = 64,
                .frameDelay = 0.1f,
                .textureId = make_nnTex("animation-template"),
            };
            const auto entity = DestroyEntity(name, type, true);
            if (entity != nullptr && !isInitialSync) {
               entitiesManager.SpawnEntity<World::Effect>(entity->GetPosition().x, entity->GetPosition().y, 64, 64, animation);
            }
        }

        for (const auto& [entityKey, locations] : state.registeredEntities) {
            const auto name = entityKey.substr(0, entityKey.find('.'));
            const auto type = entityKey.substr(entityKey.find('.') + 1);
            if (locations.contains(locationName)) {
                Renderer::AnimationHandle animation = Renderer::AnimationHandle {
                    .numOfFrames = 16,
                    .maxElementsPerRow = 4,
                    .frameSize = 64,
                    .frameDelay = 0.1f,
                    .textureId = make_nnTex("animation-template"),
                };
                const auto entity = SpawnEntity(name, type, true); 
                if (entity != nullptr && !isInitialSync) {
                    entitiesManager.SpawnEntity<World::Effect>(entity->GetPosition().x, entity->GetPosition().y, 64, 64, animation);
                }
            }
        }

        isInitialSync = false;

    }

    void OnWorldStateUpdated(WorldStateUpdatedEvent& event) {
        Sync();
    }

    [[nodiscard]] bool LoadMap(const std::string& filename) {
        // Load JSON map data
        std::string filepath = filename;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Err(std::format("Failed to load map: {}", filepath));
            return false;
        }

        isInitialSync = true;

        Physics::Reset();
        UnloadCurrentMap();
        currentLevel = filename;

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        mapData = nlohmann::json::parse(buffer.str());

        // Load tiles from JSON
        World::ETiles* tiles = entitiesManager.SpawnEntity<World::ETiles>(0, 0, 0, 0);
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

        if (mapData.contains("colliders") && mapData["colliders"].is_array()) {
            for (const auto& colliderJson : mapData["colliders"]) {
                const float x = colliderJson["x"].get<float>();
                const float y = colliderJson["y"].get<float>();
                const float width = colliderJson["width"].get<float>();
                const float height = colliderJson["height"].get<float>();
                Physics::CreateStaticRectangle(x, y, width, height);
            }
        }

        if (mapData.contains("colliders_circles") && mapData["colliders_circles"].is_array()) {
            for (const auto& colliderCircleJson : mapData["colliders_circles"]) {
                const float x = colliderCircleJson["x"].get<float>();
                const float y = colliderCircleJson["y"].get<float>();
                const float radius = colliderCircleJson["r"].get<float>();
                Physics::CreateStaticCircle(x, y, radius);
            }
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
                spawners.push_back({ npcName, "vil", x, y, shouldSpawnInstantly });
//                World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
//                npc->SetTagName(std::format("{}.vil", npcName));
            }
        }

        // Load interactible_objects into spawners before LoadSpawners (must be in spawners for SeedInstantSpawn + SpawnEntity)
        if (mapData.contains("interactible_objects") && mapData["interactible_objects"].is_array()) {
            for (const auto& interactableObjectJson : mapData["interactible_objects"]) {
                const std::string interactableObjectName = interactableObjectJson["name"].get<std::string>();
                const float x = interactableObjectJson["x"].get<float>();
                const float y = interactableObjectJson["y"].get<float>();
                const float width = interactableObjectJson["width"].get<float>();
                const float height = interactableObjectJson["height"].get<float>();
                World::EInteractableObject* interactableObject = entitiesManager.SpawnEntity<World::EInteractableObject>(x, y, width, height, interactableObjectName);
            }
        }

        World::ESpawners* spawnersEntity = entitiesManager.SpawnEntity<World::ESpawners>(0, 0, 0, 0);
        spawnersEntity->LoadSpawners(spawners);
        
        // Load interactables from JSON (direct spawn, not via spawners)
        if (mapData.contains("interactables") && mapData["interactables"].is_array()) {
            for (const auto& interactableJson : mapData["interactables"]) {
                const std::string interactableName = interactableJson["name"].get<std::string>();
                const float x = interactableJson["x"].get<float>();
                const float y = interactableJson["y"].get<float>();
                const float width = interactableJson["width"].get<float>();
                const float height = interactableJson["height"].get<float>();
                World::EInteractable* interactable = entitiesManager.SpawnEntity<World::EInteractable>(x, y, width, height, make_nnInteractId(interactableName));
                interactable->LoadData(make_nnTex(interactableJson["texture"].get<std::string>()), width, height);
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
                const std::string spawnPointMatch = levelChangeTriggerJson["spawnPointMatch"].get<std::string>();
                const bool tease = levelChangeTriggerJson.contains("tease") ? levelChangeTriggerJson["tease"].get<bool>() : false;
                const std::string voidTextureId = levelChangeTriggerJson["texture"].get<std::string>();
                World::ETriggerLocation* triggerLocation = entitiesManager.SpawnEntity<World::ETriggerLocation>(x, y, width, height, locationName, spawnPointMatch, voidTextureId, tease);
                // Возможно триггеру не нужна будет текстура в конце концов
                // Хотя это может быть, например, обьект телепорт. Почему бы и нет
            }
        }

        // Load spawn points from JSON (name -> position)
        spawnPointsByName.clear();
        if (mapData.contains("spawnPoints") && mapData["spawnPoints"].is_array()) {
            for (const auto& spJson : mapData["spawnPoints"]) {
                if (!spJson.is_object() || !spJson.contains("name") || !spJson.contains("x") || !spJson.contains("y")) {
                    continue;
                }
                std::string name = spJson["name"].get<std::string>();
                float x = spJson["x"].get<float>();
                float y = spJson["y"].get<float>();
                spawnPointsByName[name] = { x, y };
            }
        }

        // Load players from JSON
        if (mapData.contains("players") && mapData["players"].is_array()) {
            for (const auto& playerJson : mapData["players"]) {
                const std::string playerName = playerJson["name"].get<std::string>();
                const float x = playerJson["x"].get<float>();
                const float y = playerJson["y"].get<float>();
                World::EPlayer* player = entitiesManager.SpawnEntity<World::EPlayer>(x, y, 0, 0, playerName);
                World::Camera::instance().Follow(player);
                World::Camera::instance().SetPosition(x, y); // Нужно чтобы не было небольшого смещения камеры при старте уровня
            }
        }

        // Load always on top tiles from JSON
        World::ETiles* alwaysOnTopTiles = entitiesManager.SpawnEntity<World::ETiles>(0, 0, 0, 0);
        if (mapData.contains("alwaysOnTop") && mapData["alwaysOnTop"].is_array()) {
            std::vector<World::ETiles::Tile> alwaysOnTopTilesData;
            for (const auto& tileJson : mapData["alwaysOnTop"]) {
                World::ETiles::Tile tile;
                tile.x = tileJson["x"].get<float>();
                tile.y = tileJson["y"].get<float>();
                tile.width = tileJson["width"].get<float>();
                tile.height = tileJson["height"].get<float>();
                std::string textureName = tileJson["texture"].get<std::string>();
                tile.texture = make_nnTex(textureName);
                tile.tiled = tileJson.contains("tiled") ? tileJson["tiled"].get<bool>() : false;
                alwaysOnTopTilesData.push_back(tile);
            }
            alwaysOnTopTiles->LoadTiles(alwaysOnTopTilesData);
        }

        // Тут первый раз создаем все Entity на локации при старте игры (в файлавой директории)
        const std::string locationName = GetCurrentMapName();
        RevealLocationInFilesystem(locationName);
        SeedInstantSpawnEntitiesInFilesystem();
        EventBus::instance().EmitEvent<ClearWorldStateEvent>();
        EventBus::instance().EmitEvent<WindowFocusedEvent>();

        EventBus::instance().EmitEvent<LocationChangedEvent>(locationName);
        UpdateCameraScaleFactor(locationName);

        // HACK: Чтобы перезагрузить состояние мира после загрузки локации
        return true;
    }

    [[nodiscard]] bool LoadLastLoadedLevel() {
        bool result = LoadMap(ProgressSystemManager::Player().lastLevel);
        if (result) {
            if (World::EPlayer* player = entitiesManager.GetEntitiesContainer().FindEntity<World::EPlayer>(); player != nullptr) {
                player->SetPosition(ProgressSystemManager::Player().position.x, ProgressSystemManager::Player().position.y);
            }
        }
        return result;
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
        prevInteractedEntities = newInteractedEntities;
        newInteractedEntities.clear();
        entitiesManager.Update(deltaTime);
        World::Camera::instance().Update(deltaTime);
    }

    void Render(float deltaTime) {
        entitiesManager.Render(deltaTime);
    }

    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer() {
        return entitiesManager.GetEntitiesContainer();
    }

    void UpdateCameraScaleFactor(const std::string& locationName) {
        if (locationName == "backroad") {
            World::Camera::instance().SetScaleFactor(1.7f);
        }
        if (locationName == "intro") {
            World::Camera::instance().SetScaleFactor(1.1f);
        }
        if (locationName == "world-void") {
            World::Camera::instance().SetScaleFactor(1.5f);
        }
        if (locationName == "crossroads") {
            World::Camera::instance().SetScaleFactor(1.4f);
        }
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

            //objectsLocations[GetCurrentMapName()].insert({ { name, type } });
            const auto position = optPosition.value();
            if (type == "vil") {
                auto npc = entitiesManager.SpawnEntity<World::ENpc>(position.x, position.y, 0, 0, name);
                npc->SetTagName({ name, type });
                Logger::Log(std::format("[Map] Spawned NPC: {} of type {}", name, type));
                return npc;
            }
            if (type == "object") {
                auto obj = entitiesManager.SpawnEntity<World::EInteractableObject>(position.x, position.y, 0, 0, name);
                obj->SetTagName({ name, type });
                Logger::Log(std::format("[Map] Spawned interactable object: {} of type {}", name, type));
                return obj;
            }
            if (type == "box") {
                auto box = entitiesManager.SpawnEntity<World::EBox>(position.x, position.y, 0, 0, make_nnBoxName(name));
                box->SetTagName({ name, type });
                return box;
            }
            if (type == "pit" && !removedEntities.contains(std::format("{}.{}", name, type))) {
                auto pit = entitiesManager.SpawnEntity<World::EPit>(position.x, position.y, 0, 0, make_nnBoxName(name));
                pit->SetTagName({ name, type });
                return pit;
            }
        }

//        if (type == "vil") {
//            if (mapData.contains("npcs") && mapData["npcs"].is_array()) {
//                for (const auto& npcJson : mapData["npcs"]) {
//                    const std::string npcName = npcJson["name"].get<std::string>();
//                    if (npcName == name) {
//                        const float x = npcJson["x"].get<float>();
//                        const float y = npcJson["y"].get<float>();
//                        World::ENpc* npc = entitiesManager.SpawnEntity<World::ENpc>(npcName, x, y);
//                        npc->SetTagName(std::format("{}.vil", npcName));
//                        return npc;
//                    }
//                }
//            }
//        }
        return nullptr;
    }

    World::Entity* DestroyEntity(const std::string& name, const std::string& type, bool fromWorldState) {
        if (!type.empty() && !name.empty()) {
            auto entity = entitiesManager.GetEntitiesContainer().FindEntity({ name, type });
            if (entity) {
                if (fromWorldState) {
                    EventBus::instance().EmitEvent<EntityDestroyedEvent>(name, type);
                }
                entity->Destroy();
                removedEntities.insert(std::format("{}.{}", name, type));

                return entity;
            }
        }
        return nullptr;
    }

    void OpenCurrentLocationInExplorer() {
        const auto locationName = GetCurrentMapName();
        if (locationName == "intro") {
            return;
        }
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
        
        const auto& state = WorldState::GetCurrentState();
        for (const auto& [name, type] : instantSpawns) {
            const auto key = std::format("{}.{}", name, type);
            if (state.registeredEntities.contains(key)) {
                if (state.registeredLocations.contains(locationName)) {
                    continue;
                }
                WorldState::RegisterInWorldState({ name, type });
                continue;
            }
            SpawnEntity(name, type, true);
        }
        Logger::Log(std::format("Seeded {} instant-spawn entities in village/{}", instantSpawns.size(), locationName));
    }

    std::string GetCurrentMapName() const {
        return std::filesystem::path(currentLevel).filename().stem().string();
    }

    void RevealLocationInFilesystem(const std::string& locationName) {
        if (locationName == "world-void" || locationName == "intro") {
            return;
        }
        const auto villageDir = std::filesystem::path("village") / locationName;
        FileSystemManager::CreateDirectory(villageDir.string());
    }

    void SetPlayerPositionToSpawnPoint(const std::string& spawnPointName) {
        auto it = spawnPointsByName.find(spawnPointName);
        if (it == spawnPointsByName.end()) {
            Logger::Warn(std::format("[Map] Spawn point '{}' not found", spawnPointName));
            return;
        }
        World::EPlayer* playerEntity = entitiesManager.GetEntitiesContainer().FindEntity<World::EPlayer>();
        if (!playerEntity) {
            Logger::Warn("[Map] Player not found, cannot set spawn point position");
            return;
        }
        const float x = it->second.first;
        const float y = it->second.second;
        playerEntity->SetPosition(x, y);
        World::Camera::instance().SetPosition(x, y);
    }

    void MarkAsInteracted(const World::Entity::TagName& tagName) {
        newInteractedEntities.insert(tagName);
    }

    const std::set<World::Entity::TagName>& GetInteractedEntities() {
        return prevInteractedEntities;
    }   

    void SetRemovedEntities(const std::set<std::string>& removedEntities) {
        this->removedEntities = removedEntities;
    }

    const std::set<std::string>& GetRemovedEntities() {
        return this->removedEntities;
    }

private:
    std::set<std::string> removedEntities;
    World::EntitiesManager entitiesManager;
    World::EPlayer* player = nullptr;
    std::string currentLevel;

    nlohmann::json mapData;
    std::map<std::string, std::pair<float, float>> spawnPointsByName;

    Events::Handler onWorldStateUpdated;
    std::set<World::Entity::TagName> newInteractedEntities;
    std::set<World::Entity::TagName> prevInteractedEntities;

    bool isInitialSync = true;
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
        Map::instance().DestroyEntity(name, type, true);
    }

    void SetPlayerPositionToSpawnPoint(const std::string& spawnPointName) {
        Map::instance().SetPlayerPositionToSpawnPoint(spawnPointName);
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

    void Initialize() {
        Map::instance().Initialize();
    }

    void Destroy() {
        Map::instance().Destroy();
    }

    void MarkAsInteracted(const World::Entity::TagName& tagName) {
        Map::instance().MarkAsInteracted(tagName);
    }

    const std::set<World::Entity::TagName>& GetInteractedEntities() {
        return Map::instance().GetInteractedEntities();
    }

    void SetRemovedEntities(const std::set<std::string>& removedEntities) {
        Map::instance().SetRemovedEntities(removedEntities);
    }

    const std::set<std::string>& GetRemovedEntities() {
        return Map::instance().GetRemovedEntities();
    }

};
