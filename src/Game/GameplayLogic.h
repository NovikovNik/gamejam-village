#pragma once

#include <string_view>
#include <vector>

using GameActId = std::string_view;

// Все акты задаются здесь!!

namespace GameplayEntities {
    constexpr std::string_view Cow = "cow";
    constexpr std::string_view Joe = "joe";
    constexpr std::string_view Elder = "elder";
    constexpr std::string_view Sign = "Road_Sign";
    constexpr std::string_view Guard = "guard";
    constexpr std::string_view Cat = "cat";
    constexpr std::string_view ChestBox = "Chestbox";
    constexpr std::string_view Sword = "Sword";
}
namespace GameActIds {
    constexpr GameActId Intro    = "intro";
    constexpr GameActId Tutorial = "tutorial";
    constexpr GameActId Main     = "main";

    inline std::vector<GameActId> GetAllGameActIds() { return {Intro, Tutorial, Main}; };
}

namespace GameplayLogic {
    void Initialize();
    void LoadGameAct(GameActId id);
    std::string GetCurrentGameActId();
    void Destroy();
    // Для обновления событий внутри конкретного одного акта
    void Update(float deltaTime);
    // Для переходов на следующие акты
    void UpdateCurrentGameAct();
}
