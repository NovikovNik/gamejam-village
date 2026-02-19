#pragma once

#include <string>

namespace GameplayLogic {
    void Initialize();
    void LoadGameAct(const std::string& gameActName);
    void Destroy();
    void Update();
}
