#pragma once
#include <Entities/EntitiesManager.h>
#include <string>

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename);
    [[nodiscard]] std::string GetCurrentMapName();
    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer();
    [[nodiscard]] World::Entity* SpawnEntity(const std::string& name, const std::string& type);
    [[nodiscard]] bool LoadLastLoadedLevel();
    void DestroyEntity(const std::string& name, const std::string& type);

    void Update(float deltaTime);
    void Render(float deltaTime);
    void ReloadMap();
    void UnloadCurrentMap();
    void OpenCurrentLocationInExplorer();
    void SeedInstantSpawnEntitiesInFilesystem();
    void SetPlayerPositionToSpawnPoint(const std::string& spawnPointName);
};
