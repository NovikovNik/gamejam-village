#include "WorldState.h"
#include <FileSystem/FileSystem.h>
#include <EventBus/EventBus.h>
#include <Events/WindowFocusedEvent.h>
#include <Events/ClearWorldStateEvent.h>
#include <Events/ChangeLocationEvent.h>
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
            [[maybe_unused]] const auto entity = MapManager::SpawnEntity(change.name, change.type);
            Logger::Log(std::format("[WorldState] Added object: {} of type {}", change.name, change.type));
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
        Logger::Log(std::format("[WorldState] Changing location to: {}", event.locationPath));
        MapManager::LoadMap(event.locationPath);
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
                        obj.name = fileEntry.path().stem().string();  // filename without extension
                        obj.type = fileEntry.path().extension().string();
                        // Remove the leading dot from extension
                        if (!obj.type.empty() && obj.type[0] == '.') {
                            obj.type = obj.type.substr(1);
                        }
                        if (!obj.type.empty()) {  // skip files without extension (e.g. "villager")
                            subfolderObjects.push_back(obj);
                        }
                    }
                }
                
                currentState[subfolderName] = subfolderObjects;
            }
        }
        
        Logger::Log("Loaded game state from village folder");
    }

private:
    LocationsStates::State currentState;
    LocationsStates::State lastSeenState;

    PersistentState persistentState;

    Events::Handler onWindowFocused;
    Events::Handler onClearWorldState;
    Events::Handler onChangeLocation;
};

void WorldState::Initiate() {
    GameStateManager::instance().Initiate();
}

LocationsStates::LocationChanges WorldState::SyncLocationAndGetChanges(const LocationsStates::LocationName& locationName) { 
    return GameStateManager::instance().SyncLocationAndGetChanges(locationName);
}

void WorldState::Destroy() {
    GameStateManager::instance().Destroy();
}