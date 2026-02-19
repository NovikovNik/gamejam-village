#include "ProgressSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include "AppDataSaveHelper.h"
#include <Entities/EPlayer.h>
#include <glm/glm.hpp>
#include "PlayerSaveData.h"
#include <libs/json/single_include/nlohmann/json.hpp>

class ProgressSystem: public Singleton<ProgressSystem> {
    public:
        // Основные системные функции
        void Initialize() {
            if (!AppDataSaveHelper::GameSaveExists()) {
                Logger::Log("[ProgressSystem] No save file found, creating new save file");
                // Инициализация данных по умолчанию
                playerSaveData.ResetToDefaults();
                SaveData();
            }
            else {
                Logger::Log("[ProgressSystem] Save file found, loading save file");
                LoadData();
            }
            Logger::Log("[ProgressSystem] Initialized");
        }

        void SaveData() {
            nlohmann::json j;
            playerSaveData.ToJson(j);
            AppDataSaveHelper::SaveGameJson(j);
            dirty = false;
            Logger::Log("[ProgressSystem] Data saved");
        }

        void LoadData() {
            nlohmann::json j; 
            if (!AppDataSaveHelper::LoadGameJson(j)) {
                Logger::Warn("[ProgressSystem] Failed to load save file, using defaults");
                playerSaveData.ResetToDefaults();
                return;
            }
            playerSaveData.FromJson(j);
            Logger::Log("[ProgressSystem] Data loaded");
        }

    public:
        PlayerSaveData& Player() { 
            dirty = true;
            return playerSaveData; 
        }

    private:
        bool dirty = false;
        PlayerSaveData playerSaveData;
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

PlayerSaveData& ProgressSystemManager::Player() {
    return ProgressSystem::instance().Player();
}