#pragma once

#include "PlayerSaveData.h"
#include "AudioSaveData.h"
#include "WorldSaveData.h"

namespace ProgressSystemManager {
    void Initialize();
    void SaveData();
    void LoadData();

    PlayerSaveData& Player();
    AudioSaveData&  Audio();
    WorldSaveData&  World();
}