#pragma once

#include <string_view>
#include <vector>

using GameActId = std::string_view;

// Все акты задаются здесь!!
namespace GameActIds {
    constexpr GameActId Intro    = "intro";
    constexpr GameActId Tutorial = "tutorial";
    constexpr GameActId Act1     = "act1";

    inline std::vector<GameActId> GetAllGameActIds() { return {Intro, Tutorial, Act1}; };
}

namespace GameplayLogic {
    void Initialize();
    void LoadGameAct(GameActId id);
    std::string GetCurrentGameActId();
    void Destroy();
    void Update();
}
