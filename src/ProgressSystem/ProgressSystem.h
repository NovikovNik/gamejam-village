#pragma once

#include "PlayerSaveData.h"

namespace ProgressSystemManager {
    void Initialize();
    void SaveData();
    void LoadData();

    // Прямой доступ к данным сохранения игрока
    PlayerSaveData& Player();
}