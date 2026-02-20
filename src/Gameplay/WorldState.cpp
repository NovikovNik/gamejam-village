#include "WorldState.h"
#include <FileSystem/FileSystem.h>
#include <EventBus/EventBus.h>
#include <Events/WindowFocusedEvent.h>
#include <Events/ClearWorldStateEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Logger/Logger.h>
#include <Map/Map.h>
#include <Utils/Singleton.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <filesystem>
#include <format>

struct PersistentState
{
    std::map<std::string, std::string> mapObjectsLocations;
    std::set<std::string> existingLevels;
};

struct WorldBlackboard
{
    struct VillagerElder
    {
        int conversationTutorialPreRemoved = 0;
        int conversationTutorialPostRemoved = 0;
    };

    VillagerElder villagerElder;
};

struct NextLocation
{
    std::string locationPath;
    std::string spawnPoint;
};

class GameStateManager: public Singleton<GameStateManager>
{
public:
    void Initiate()
    {
        onWindowFocused = EventBus::instance().SubscribeToEvent<WindowFocusedEvent>(this, &GameStateManager::OnWindowFocused);
        onClearWorldState = EventBus::instance().SubscribeToEvent<ClearWorldStateEvent>(this, &GameStateManager::OnClearWorldState);
        onChangeLocation = EventBus::instance().SubscribeToEvent<ChangeLocationEvent>(this, &GameStateManager::OnChangeLocation);
    }

    void Destroy()
    {
        onWindowFocused.Destroy();
        onClearWorldState.Destroy();
        onChangeLocation.Destroy();
    }

    void Update()
    {
        if (!nextLocation.has_value())
        {
            return;
        }

        Logger::Log(std::format("[WorldState] Changing location to: {}", nextLocation->locationPath));
        bool isLoaded = MapManager::LoadMap(nextLocation->locationPath);
        if (isLoaded) {
            if (!nextLocation->spawnPoint.empty()) {
                MapManager::SetPlayerPositionToSpawnPoint(nextLocation->spawnPoint);
            }
            ProgressSystemManager::Player().lastLevel = nextLocation->locationPath;
            ProgressSystemManager::SaveData();
        }

        nextLocation.reset();
    }

    void OnWindowFocused(WindowFocusedEvent&)
    {
        Logger::Log("[WorldState] Window focused");
        LoadNewGameStateFromFilesystem();

        const auto locationName = MapManager::GetCurrentMapName();
        const auto& changes = SyncLocationAndGetChanges(locationName);
        for (const auto& change : changes.added) {
            if (change.type.empty()) {
                Logger::Warn(std::format("[WorldState] Skipping object '{}': empty type (use name.type -> Spaghetti.villager)", change.name));
                continue;
            }
            const auto key = std::format("{}.{}", change.name, change.type);
            persistentState.mapObjectsLocations[key] = locationName;
            const auto entity = MapManager::SpawnEntity(change.name, change.type);
            if (entity != nullptr) {
                Logger::Log(std::format("[WorldState] Added object: {} of type {}", change.name, change.type));
            }
            else {
                Logger::Warn(std::format("[WorldState] Failed to spawn object: {} of type {}", change.name, change.type));
            }
        }
        for (const auto& change : changes.removed) {
            const auto key = std::format("{}.{}", change.name, change.type);
            if (persistentState.mapObjectsLocations.contains(key)) {
                persistentState.mapObjectsLocations.erase(key);
            }

            MapManager::DestroyEntity(change.name, change.type);

//            const auto entity = MapManager::GetEntitiesContainer().FindEntity(std::format("{}.{}", change.name, change.type));
//            if (entity) {
//                entity->Destroy();
//            }
            Logger::Log(std::format("[WorldState] Removed object: {} of type {}", change.name, change.type));
        }
    }

    void OnClearWorldState(ClearWorldStateEvent&)
    {
        Logger::Log("[WorldState] Clearing world state");
        currentState.clear();
        lastSeenState.clear();
    }

    // Обработчик перехода на новый уровень
    // Словно ему место не здесь и не в Map.cpp, но пока живет тут
    void OnChangeLocation(ChangeLocationEvent& event)
    {
        nextLocation = NextLocation{
            .locationPath = event.locationPath,
            .spawnPoint = event.spawnPoint,
        };
    }

    [[nodiscard]] LocationsStates::LocationChanges SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName)
    {
        LocationsStates::LocationChanges changes;
        const auto& currentLocationObjects = currentState[locationName];
        const auto& lastSeenLocationObjects = lastSeenState[locationName];
        for (const auto& object : currentLocationObjects) {
            if (std::find(lastSeenLocationObjects.begin(), lastSeenLocationObjects.end(), object) == lastSeenLocationObjects.end()) {
                changes.added.push_back(object);
            }
        }
        for (const auto& object : lastSeenLocationObjects) {
            if (std::find(currentLocationObjects.begin(), currentLocationObjects.end(), object) == currentLocationObjects.end()) {
                changes.removed.push_back(object);
            }
        }
        lastSeenState[locationName] = currentLocationObjects;
        return changes;
    }

    [[nodiscard]] const LocationsStates::State& GetCurrentState() const { return currentState; }
    [[nodiscard]] const LocationsStates::State& GetLastSeenState() const { return lastSeenState; }

    void LoadNewGameStateFromFilesystem()
    {
        namespace fs = std::filesystem;
        
        // Find the village folder
        fs::path villagePath = FileSystemManager::GetExecutableDir() / "village";
        
        if (!fs::exists(villagePath) || !fs::is_directory(villagePath)) {
            Logger::Log("Village folder not found!");
            return;
        }
        
        // Clear current state
        currentState.clear();
        
        // Process root village folder files
        LocationsStates::LocationObjects rootObjects;
        for (const auto& entry : fs::directory_iterator(villagePath)) {
            if (entry.is_regular_file()) {
                LocationsStates::Object obj;
                obj.name = entry.path().stem().string();  // filename without extension
                obj.type = entry.path().extension().string();
                // Remove the leading dot from extension
                if (!obj.type.empty() && obj.type[0] == '.') {
                    obj.type = obj.type.substr(1);
                }
                if (!obj.type.empty()) {
                    rootObjects.push_back(obj);
                }
            }
        }
        currentState[""] = rootObjects;
        
        // Process subfolders (depth 1)
        for (const auto& entry : fs::directory_iterator(villagePath)) {
            if (entry.is_directory()) {
                std::string subfolderName = entry.path().filename().string();
                LocationsStates::LocationObjects subfolderObjects;
                
                // Scan files inside this subfolder
                for (const auto& fileEntry : fs::directory_iterator(entry.path())) {
                    if (fileEntry.is_regular_file()) {
                        LocationsStates::Object obj;
                        
                        // Handle files with pattern: <name>.<type>.txt
                        auto originalFilePath = fileEntry.path();
                        auto filePath = originalFilePath;
                        bool wasTextFile = false;
                        
                        // If the file ends with .txt, remove it first
                        if (filePath.extension() == ".txt") {
                            filePath = filePath.stem();  // Remove .txt, get <name>.<type>
                            wasTextFile = true;
                        }
                        
                        // Now extract name and type from <name>.<type>
                        obj.name = filePath.stem().string();  // filename without extension
                        obj.type = filePath.extension().string();
                        
                        // Remove the leading dot from extension
                        if (!obj.type.empty() && obj.type[0] == '.') {
                            obj.type = obj.type.substr(1);
                        }
                        
                        if (!obj.type.empty()) {  // skip files without extension (e.g. "villager")
                            // Migrate old .txt files to new format
                            if (wasTextFile) {
                                auto newFilePath = originalFilePath.parent_path() / (obj.name + "." + obj.type);
                                
                                // Delete old .txt file
                                try {
                                    fs::remove(originalFilePath);
                                    Logger::Log(std::format("[WorldState] Deleted old file: {}", 
                                        originalFilePath.filename().string()));
                                } catch (const std::exception& e) {
                                    Logger::Err(std::format("[WorldState] Failed to remove old file: {}", e.what()));
                                }
                                
                                // Create new file without .txt extension
                                if (!fs::exists(newFilePath)) {
                                    try {
                                        FileSystemManager::CreateKeyFile(
                                            originalFilePath.parent_path().string(), 
                                            obj.name + "." + obj.type
                                        );
                                        Logger::Log(std::format("[WorldState] Created new file: {}", 
                                            newFilePath.filename().string()));
                                    } catch (const std::exception& e) {
                                        Logger::Err(std::format("[WorldState] Failed to create new file: {}", e.what()));
                                    }
                                }
                            }
                            
                            subfolderObjects.push_back(obj);
                        }
                    }
                }
                
                currentState[subfolderName] = subfolderObjects;
            }
        }
        
        Logger::Log("Loaded game state from village folder");
    }

    void AddToWorldState(const World::Entity::TagName& tagName) {

        const auto locationName = MapManager::GetCurrentMapName();
        const auto locationObjects = lastSeenState.find(locationName);
        if (locationObjects == lastSeenState.end()) {
            return;
        }

        const auto object = std::find_if(locationObjects->second.begin(), locationObjects->second.end(), [&tagName](const LocationsStates::Object& object) {
            return object.name == tagName.name && object.type == tagName.type;
        });
        if (object != locationObjects->second.end()) {
            return;
        }
        
        const auto villageDir = std::filesystem::path("village") / locationName;
        const auto filename = std::format("{}.{}", tagName.name, tagName.type);
        
        // Delete old .txt file if it exists (migration from old format)
        const auto oldFilenameWithTxt = filename + ".txt";
        FileSystemManager::DeleteKeyFile(villageDir.string(), oldFilenameWithTxt);
        
        // Create new file without .txt extension
        FileSystemManager::CreateKeyFile(villageDir.string(), filename);

        lastSeenState[locationName].push_back(LocationsStates::Object{
            .name = tagName.name,
            .type = tagName.type,
        });
    }

    void RemoveFromWorldState(const World::Entity::TagName& tagName) {
        const auto locationName = MapManager::GetCurrentMapName();

        const auto villageDir = std::filesystem::path("village") / locationName;
        const auto filename = std::format("{}.{}", tagName.name, tagName.type);
        
        // Delete both old .txt format and new format (whichever exists)
        const auto oldFilenameWithTxt = filename + ".txt";
        FileSystemManager::DeleteKeyFile(villageDir.string(), oldFilenameWithTxt);
        FileSystemManager::DeleteKeyFile(villageDir.string(), filename);
        
        lastSeenState[locationName].erase(std::remove_if(lastSeenState[locationName].begin(), lastSeenState[locationName].end(), [&tagName](const LocationsStates::Object& object) {
            return object.name == tagName.name && object.type == tagName.type;
        }), lastSeenState[locationName].end());
    }

private:
    LocationsStates::State currentState;
    LocationsStates::State lastSeenState;

    PersistentState persistentState;

    Events::Handler onWindowFocused;
    Events::Handler onClearWorldState;
    Events::Handler onChangeLocation;

    std::optional<NextLocation> nextLocation;
};

void WorldState::Initiate() {
    GameStateManager::instance().Initiate();
}

LocationsStates::LocationChanges WorldState::SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName) { 
    return GameStateManager::instance().SyncLocationAndGetChanges(locationName);
}

const LocationsStates::State& WorldState::GetCurrentState() {
    return GameStateManager::instance().GetCurrentState();
}

const LocationsStates::State& WorldState::GetLastSeenState() {
    return GameStateManager::instance().GetLastSeenState();
}

void WorldState::Destroy() {
    GameStateManager::instance().Destroy();
}

void WorldState::AddToWorldState(const World::Entity::TagName& tagName) {
    GameStateManager::instance().AddToWorldState(tagName);
}

void WorldState::RemoveFromWorldState(const World::Entity::TagName& tagName) {
    GameStateManager::instance().RemoveFromWorldState(tagName);
}

void WorldState::Update() {
    GameStateManager::instance().Update();
}
