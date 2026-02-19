#pragma once

#include <glm/glm.hpp>
#include <string>

namespace ProgressSystemManager {
    void Initialize();
    void SaveData();
    void LoadData();

    // Getters for save related variables
    glm::vec2 GetPlayerPosition();
    void SetPlayerPosition(const glm::vec2& position);
    std::string GetLastLoadedLevel();
    void SetLastLoadedLevel(const std::string& level);
}