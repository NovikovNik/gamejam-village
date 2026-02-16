#pragma once
#include <string>

namespace MapManager {
    [[nodiscard]] bool LoadMap(const std::string& filename);

    void Update(float deltaTime);
    void Render(float deltaTime);
};
