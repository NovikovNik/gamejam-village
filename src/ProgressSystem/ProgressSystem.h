#pragma once

#include "PlayerSaveData.h"
#include "AudioSaveData.h"

namespace ProgressSystemManager {
    void Initialize();
    void SaveData();
    void LoadData();

    PlayerSaveData& Player();
    AudioSaveData&  Audio();
}