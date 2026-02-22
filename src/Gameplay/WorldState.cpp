#include "WorldState.h"
#include <FileSystem/FileSystem.h>
#include <EventBus/EventBus.h>
#include <Events/WindowFocusedEvent.h>
#include <Events/ClearWorldStateEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/WorldStateEvents.h>
#include <Events/EntitiesEvent.h>
#include <Events/LocationChangedEvent.h>
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
        Sync();
        onWindowFocused = EventBus::instance().SubscribeToEvent<WindowFocusedEvent>(this, &GameStateManager::OnWindowFocused);
        onClearWorldState = EventBus::instance().SubscribeToEvent<ClearWorldStateEvent>(this, &GameStateManager::OnClearWorldState);
        onChangeLocation = EventBus::instance().SubscribeToEvent<ChangeLocationEvent>(this, &GameStateManager::OnChangeLocation);
        onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &GameStateManager::OnEntityDestroyed);
        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &GameStateManager::OnLocationChanged);
    }

    void Destroy()
    {
        onWindowFocused.Destroy();
        onClearWorldState.Destroy();
        onChangeLocation.Destroy();
        onEntityDestroyed.Destroy();
        onLocationChanged.Destroy();
    }

    void OnEntityDestroyed(EntityDestroyedEvent& event)
    {
        RemoveFromWorldState(World::Entity::TagName{event.GetName(), event.GetType()});
    }

    void OnLocationChanged(LocationChangedEvent& event)
    {
        LoadNewGameStateFromFilesystem();
        EventBus::instance().EmitEvent<WorldStateUpdatedEvent>();
    }

    void Update()
    {
        if (!nextLocation.has_value())
        {
            return;
        }

        Logger::Log(std::format("[WorldState] Changing location to: {}", nextLocation->locationPath));
        
        LoadNewGameStateFromFilesystem();
        bool isLoaded = MapManager::LoadMap(nextLocation->locationPath);
        if (isLoaded) {
            if (!nextLocation->spawnPoint.empty()) {
                Logger::Log(std::format("[WorldState] Setting player position to spawn point: {}", nextLocation->spawnPoint));
                MapManager::SetPlayerPositionToSpawnPoint(nextLocation->spawnPoint);
            }
            ProgressSystemManager::Player().lastLevel = nextLocation->locationPath;
            ProgressSystemManager::SaveData();
        }


        nextLocation.reset();
    }

    void Sync()
    {
        Logger::Log("[WorldState] Window focused");
        LoadNewGameStateFromFilesystem();

        const auto currentLocation = MapManager::GetCurrentMapName();
        if (currentLocation == "world-void" && currentState.registeredLocations.contains(backupLocationPath) && currentState.registeredLocations.at(backupLocationPath)) {
            nextLocation = NextLocation{
                .locationPath = std::format("assets/maps/{}.json", backupLocationPath),
                .spawnPoint = "",
            };
            backupLocationPath.clear();
            return;
        }
        if ((currentLocation != "world-void" && currentLocation != "intro") && currentState.registeredLocations.contains(currentLocation) && !currentState.registeredLocations.at(currentLocation)) {
            nextLocation = NextLocation{
                .locationPath = "assets/maps/world-void.json",
                .spawnPoint = "",
            };
            backupLocationPath = currentLocation;
        }
    }

    void OnWindowFocused(WindowFocusedEvent&)
    {
        Sync();
        EventBus::instance().EmitEvent<WorldStateUpdatedEvent>();
    }

    void OnClearWorldState(ClearWorldStateEvent&)
    {
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

    const LocationsStates::State& GetCurrentState() const { return currentState; }
    void SetCurrentState(const LocationsStates::State& state) { currentState = state; }
    const std::map<std::string, std::set<std::string>>& GetRegisteredEntities() const { return currentState.registeredEntities; }

    void LoadNewGameStateFromFilesystem()
    {
        namespace fs = std::filesystem;
        
        // Find the village folder
        fs::path villagePath = FileSystemManager::GetExecutableDir() / "village";
        
        if (!fs::exists(villagePath) || !fs::is_directory(villagePath)) {
            Logger::Log("Village folder not found!");
            return;
        }
        
        std::set<std::string> registeredLocations;
        for (const auto& [locationName, isRegistered] : currentState.registeredLocations) {
            registeredLocations.insert(locationName);
        }

        std::set<std::string> registeredEntities;
        for (const auto& [entityKey, locations] : currentState.registeredEntities) {
            registeredEntities.insert(entityKey);
        }

        // Clear current state
        currentState = {};
        // Process subfolders (depth 1)
        for (const auto& entry : fs::directory_iterator(villagePath)) {
            if (entry.is_directory()) {
                std::string subfolderName = entry.path().filename().string();
                
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
                        
                        if (!obj.type.empty()) {  // skip files without extension (e.g. "vil")
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
                            
                            currentState.registeredEntities[std::format("{}.{}", obj.name, obj.type)].insert(subfolderName);
                        }
                    }
                }
                currentState.registeredLocations[subfolderName] = true;
            }
        }

        for (const auto& entityKey : registeredEntities) {
            currentState.registeredEntities.try_emplace(entityKey, std::set<std::string>{});
        }

        for (const auto& locationName : registeredLocations) {
            currentState.registeredLocations.try_emplace(locationName, false);
        }
        
        Logger::Log("Loaded game state from village folder");
    }

    void RegisterInWorldState(const World::Entity::TagName& tagName) {
        if (tagName.type == "object") {
            return;
        }
        const auto key = std::format("{}.{}", tagName.name, tagName.type);

        const auto locationName = MapManager::GetCurrentMapName();
        auto& entity = currentState.registeredEntities[key];
        if (entity.contains(locationName) || locationName == "world-void" || locationName == "intro") {
            return;
        }
        entity.insert(locationName);

        const auto villageDir = std::filesystem::path("village") / locationName;
        const auto filename = std::format("{}.{}", tagName.name, tagName.type);
        
        // Delete old .txt file if it exists (migration from old format)
        const auto oldFilenameWithTxt = filename + ".txt";
        FileSystemManager::DeleteKeyFile(villageDir.string(), oldFilenameWithTxt);
        
        // Create new file without .txt extension
        FileSystemManager::CreateKeyFile(villageDir.string(), filename);
        EventBus::instance().EmitEvent<WorldStateUpdatedEvent>();
    }

    void RemoveFromWorldState(const World::Entity::TagName& tagName) {
        const auto locationName = MapManager::GetCurrentMapName();

        const auto villageDir = std::filesystem::path("village") / locationName;
        const auto filename = std::format("{}.{}", tagName.name, tagName.type);
        
        // Delete both old .txt format and new format (whichever exists)
        const auto oldFilenameWithTxt = filename + ".txt";
        FileSystemManager::DeleteKeyFile(villageDir.string(), oldFilenameWithTxt);
        FileSystemManager::DeleteKeyFile(villageDir.string(), filename);
        
        const auto key = std::format("{}.{}", tagName.name, tagName.type);
        if (currentState.registeredEntities.contains(key)) {
            currentState.registeredEntities[key].erase(locationName);
        }
    }

    void SetBackupLocationPath(const std::string& locationPath) {
        backupLocationPath = locationPath;
    }

    const std::string& GetBackupLocationPath() const { return backupLocationPath; }

private:
    LocationsStates::State currentState;
    std::string backupLocationPath;

    Events::Handler onWindowFocused;
    Events::Handler onClearWorldState;
    Events::Handler onChangeLocation;
    Events::Handler onEntityDestroyed;
    Events::Handler onLocationChanged;

    std::optional<NextLocation> nextLocation;
};

void WorldState::Initiate() {
    GameStateManager::instance().Initiate();
}

void WorldState::Destroy() {
    GameStateManager::instance().Destroy();
}

void WorldState::RegisterInWorldState(const World::Entity::TagName& tagName) {
    GameStateManager::instance().RegisterInWorldState(tagName);
}

void WorldState::Update() {
    GameStateManager::instance().Update();
}

const LocationsStates::State& WorldState::GetCurrentState() {
    return GameStateManager::instance().GetCurrentState();
}

const std::string& WorldState::GetBackupLocationPath() {
    return GameStateManager::instance().GetBackupLocationPath();
}

void WorldState::SetBackupLocationPath(const std::string& locationPath) {
    GameStateManager::instance().SetBackupLocationPath(locationPath);
}

void WorldState::SetCurrentState(const LocationsStates::State& state) {
    GameStateManager::instance().SetCurrentState(state);
}

const std::map<std::string, std::set<std::string>>& WorldState::GetRegisteredEntities() {
    return GameStateManager::instance().GetRegisteredEntities();
}

void WorldState::RemoveFromWorldState(const World::Entity::TagName& tagName) {
    GameStateManager::instance().RemoveFromWorldState(tagName);
}

