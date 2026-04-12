#include "ProgressSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include "AppDataSaveHelper.h"
#include <Entities/EPlayer.h>
#include <glm/glm.hpp>
#include "PlayerSaveData.h"
#include "AudioSaveData.h"
#include "WorldSaveData.h"
#include "QuestsSaveData.h"
#include "InventarySaveData.h"
#include <Map/Map.h>
#include <AudioSystem/AudioSystem.h>
#include <Gameplay/WorldState.h>
#include <libs/json/single_include/nlohmann/json.hpp>

class ProgressSystem: public Singleton<ProgressSystem> {
    public:
        // Основные системные функции
        void Initialize() {
            if (!AppDataSaveHelper::GameSaveExists()) {
                Logger::Log("[ProgressSystem] No save file found, creating new save file");
                // Инициализация данных по умолчанию
                playerSaveData.ResetToDefaults();
                questsSaveData.ResetToDefaults();
                SaveData();
            }
            else {
                Logger::Log("[ProgressSystem] Save file found, loading save file");
                LoadData();
            }
            Logger::Log("[ProgressSystem] Initialized");
        }

        void SaveData() {
            // Snapshot current audio state before writing.
            audioSaveData.masterVolume = AudioSystem::GetMasterVolume();
            audioSaveData.musicVolume  = AudioSystem::GetMusicVolume();
            audioSaveData.sfxVolume    = AudioSystem::GetSfxVolume();
            audioSaveData.muted        = AudioSystem::IsMuted();

            // Snapshot current world state before writing.
            worldSaveData.state = WorldState::GetCurrentState();
            worldSaveData.backupLocationPath = WorldState::GetBackupLocationPath();
            worldSaveData.removedEntities = MapManager::GetRemovedEntities();
            worldSaveData.visitedLocations = MapManager::GetVisitedLocations();
            nlohmann::json j;
            playerSaveData.ToJson(j);
            audioSaveData.ToJson(j["audio"]);
            worldSaveData.ToJson(j["world"]);
            inventorySaveData.ToJson(j["inventory"]);
            questsSaveData.ToJson(j["quests"]);
            AppDataSaveHelper::SaveGameJson(j);
            dirty = false;
            Logger::Log("[ProgressSystem] Data saved");
        }

        void LoadData() {
            nlohmann::json j;
            if (!AppDataSaveHelper::LoadGameJson(j)) {
                Logger::Warn("[ProgressSystem] Failed to load save file, using defaults");
                playerSaveData.ResetToDefaults();
                audioSaveData.ResetToDefaults();
                worldSaveData.ResetToDefaults();
                questsSaveData.ResetToDefaults();
                inventorySaveData.ResetToDefaults();
                return;
            }
            playerSaveData.FromJson(j);
            audioSaveData.FromJson(j.value("audio", nlohmann::json::object()));
            worldSaveData.FromJson(j.value("world", nlohmann::json::object()));
            questsSaveData.FromJson(j.value("quests", nlohmann::json::object()));
            inventorySaveData.FromJson(j.value("inventory", nlohmann::json::object()));
            Logger::Log("[ProgressSystem] Data loaded");
        }

    public:
        PlayerSaveData& Player() {
            dirty = true;
            return playerSaveData;
        }

        AudioSaveData& Audio() {
            dirty = true;
            return audioSaveData;
        }

        WorldSaveData& World() {
            dirty = true;
            return worldSaveData;
        }

        InventarySaveData& Inventory() {
            dirty = true;
            return inventorySaveData;
        }
        QuestsSaveData& Quests() {
            dirty = true;
            return questsSaveData;
        }

        [[nodiscard]] std::string GetPlayerLanguageSnapshot() const {
            return playerSaveData.language;
        }

    private:
        bool dirty = false;
        PlayerSaveData playerSaveData;
        AudioSaveData  audioSaveData;
        WorldSaveData  worldSaveData;
        InventarySaveData inventorySaveData;
        QuestsSaveData questsSaveData;
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

std::string ProgressSystemManager::GetPlayerLanguage() {
    return ProgressSystem::instance().GetPlayerLanguageSnapshot();
}

AudioSaveData& ProgressSystemManager::Audio() {
    return ProgressSystem::instance().Audio();
}

WorldSaveData& ProgressSystemManager::World() {
    return ProgressSystem::instance().World();
}

InventarySaveData& ProgressSystemManager::Inventory() {
    return ProgressSystem::instance().Inventory();
}

QuestsSaveData& ProgressSystemManager::Quests() {
    return ProgressSystem::instance().Quests();
}