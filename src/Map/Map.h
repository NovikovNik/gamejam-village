#pragma once
#include <Entities/EntitiesManager.h>
#include <string>
#include <set>

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename);
    [[nodiscard]] std::string GetCurrentMapName();
    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer();
    [[nodiscard]] World::Entity* SpawnEntity(const std::string& name, const std::string& type);
    [[nodiscard]] bool LoadLastLoadedLevel();
    void DestroyEntity(const std::string& name, const std::string& type);
    void MarkAsInteracted(const World::Entity::TagName& tagName);
    [[nodiscard]] const std::set<World::Entity::TagName>& GetInteractedEntities();

    void SetRemovedEntities(const std::set<std::string>& removedEntities);
    [[nodiscard]] const std::set<std::string>& GetRemovedEntities();

    void Initialize();
    void Destroy();
    void Update(float deltaTime);
    void Render(float deltaTime);
    void ReloadMap();
    void UnloadCurrentMap();
    void UpdateCameraScaleFactor(const std::string locationName);
    void OpenCurrentLocationInExplorer();
    void SeedInstantSpawnEntitiesInFilesystem();
    void SetPlayerPositionToSpawnPoint(const std::string& spawnPointName);

    void ShowInteractHint(float x, float y);
    void ShowRegistrationHint(float x, float y);
};
