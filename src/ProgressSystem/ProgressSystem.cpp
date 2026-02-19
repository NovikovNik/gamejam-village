#include "ProgressSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include "AppDataSaveHelper.h"
#include <Entities/EPlayer.h>
#include <glm/glm.hpp>


class ProgressSystem: public Singleton<ProgressSystem> {
    public:
        // Основные системные функции
        void Initialize() {
            if (!AppDataSaveHelper::GameSaveExists()) {
                Logger::Log("[ProgressSystem] No save file found, creating new save file");
                // Инициализация данных по умолчанию
                SetLastLoadedLevel("assets/maps/world-entry-2.json");
                SetPlayerPosition(glm::vec2(0.0f, 0.0f));
                SaveData();
            }
            else {
                Logger::Log("[ProgressSystem] Save file found, loading save file");
                LoadData();
            }
            Logger::Log("[ProgressSystem] Initialized");
        }

        void SaveData() {
            nlohmann::json data = GetDataToSave();
            AppDataSaveHelper::SaveGameJson(data);
            Logger::Log("[ProgressSystem] Data saved");
        }

        void LoadData() {
            AppDataSaveHelper::LoadGameJson(gameSaveData);
            // Позиция игрока
            playerPosition.x = gameSaveData["playerPosition"]["x"].get<float>();
            playerPosition.y = gameSaveData["playerPosition"]["y"].get<float>();
            // Последняя загруженная локация
            lastLoadedLevel = gameSaveData["lastLoadedLevel"].get<std::string>();
            Logger::Log("[ProgressSystem] Data loaded");
        }
    public:
        // Позиция игрока
        glm::vec2 GetPlayerPosition() {
            return playerPosition;
        }

        void SetPlayerPosition(const glm::vec2& position) {
            playerPosition = position;
        }

        // Последняя загруженная локация
        std::string GetLastLoadedLevel() const {
            return lastLoadedLevel;
        }

        void SetLastLoadedLevel(const std::string& level) {
            lastLoadedLevel = level;
        }

    private:
        // Метод для трансформации данных в json формат
        nlohmann::json GetDataToSave() {
            nlohmann::json data;
            data["playerPosition"]["x"] = static_cast<float>(playerPosition.x);
            data["playerPosition"]["y"] = static_cast<float>(playerPosition.y);
            // Последняя загруженная локация
            data["lastLoadedLevel"] = lastLoadedLevel;
            return data;
        }

    private:
        nlohmann::json gameSaveData;

        // Save related variables
        glm::vec2 playerPosition;
        std::string lastLoadedLevel;
};

void ProgressSystemManager::Initialize() {
    ProgressSystem::instance().Initialize();
}

void ProgressSystemManager::SaveData() {
    ProgressSystem::instance().SaveData();
}

void ProgressSystemManager::LoadData() {
    ProgressSystem::instance().LoadData();
}

glm::vec2 ProgressSystemManager::GetPlayerPosition() {
    return ProgressSystem::instance().GetPlayerPosition();
}

void ProgressSystemManager::SetPlayerPosition(const glm::vec2& position) {
    ProgressSystem::instance().SetPlayerPosition(position);
}

std::string ProgressSystemManager::GetLastLoadedLevel() {
    return ProgressSystem::instance().GetLastLoadedLevel();
}

void ProgressSystemManager::SetLastLoadedLevel(const std::string& level) {
    ProgressSystem::instance().SetLastLoadedLevel(level);
}