#pragma once
#include <Entities/EntitiesManager.h>
#include <string>

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename);

    [[nodiscard]] const World::EntitiesContainer& GetEntitiesContainer();

    void Update(float deltaTime);
    void Render(float deltaTime);
    void ReloadMap();
    void UnloadCurrentMap();
};
