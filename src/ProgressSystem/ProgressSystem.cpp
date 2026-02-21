#include "ProgressSystem.h"
#include <Utils/Singleton.h>
#include <Logger/Logger.h>
#include "AppDataSaveHelper.h"
#include <Entities/EPlayer.h>
#include <glm/glm.hpp>
#include "PlayerSaveData.h"
#include "AudioSaveData.h"
#include "WorldSaveData.h"
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

            nlohmann::json j;
            playerSaveData.ToJson(j);
            audioSaveData.ToJson(j["audio"]);
            worldSaveData.ToJson(j["world"]);
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
                return;
            }
            playerSaveData.FromJson(j);
            audioSaveData.FromJson(j.value("audio", nlohmann::json::object()));
            worldSaveData.FromJson(j.value("world", nlohmann::json::object()));
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

    private:
        bool dirty = false;
        PlayerSaveData playerSaveData;
        AudioSaveData  audioSaveData;
        WorldSaveData  worldSaveData;
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

AudioSaveData& ProgressSystemManager::Audio() {
    return ProgressSystem::instance().Audio();
}

WorldSaveData& ProgressSystemManager::World() {
    return ProgressSystem::instance().World();
}