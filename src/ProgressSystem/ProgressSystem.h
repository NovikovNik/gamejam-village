#pragma once

#include "PlayerSaveData.h"
#include "AudioSaveData.h"
#include "WorldSaveData.h"
#include "InventarySaveData.h"
#include "QuestsSaveData.h"

namespace ProgressSystemManager {
    void Initialize();
    void SaveData();
    void LoadData();

    PlayerSaveData& Player();
    /// Текущий язык из сохранения (без пометки dirty).
    [[nodiscard]] std::string GetPlayerLanguage();
    AudioSaveData&  Audio();
    WorldSaveData&  World();
    InventarySaveData& Inventory();
    QuestsSaveData& Quests();
}