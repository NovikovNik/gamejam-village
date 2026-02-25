#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <libs/json/single_include/nlohmann/json.hpp>

using QuestId = std::string;

enum class QuestStatus : uint8_t {
    NotStarted = 0,
    OnGoing    = 1,
    Completed  = 2,
};

namespace World {
    inline const QuestId VoidFirstEntranceQuest = "void_first_entrance_quest";
    inline const QuestId JoeCarrotQuest = "joe_carrot_quest";
    inline const QuestId JoeCarrotFinal = "joe_carrot_extra_quest"; // Отдельный квест для других строчек после вручения морковки
    inline const QuestId CowFeededQuest = "cow_feed_quest";
    inline const QuestId ElderFirstMeetingQuest = "elder_first_meeting";
    inline const QuestId ElderCatQuestInfo = "elder_cat_quest_house_info"; // Дополнительный системный квест, для отслеживания реплик для Again диалогов
    inline const QuestId ElderCatQuest = "elder_cat_quest";
    inline const QuestId ElderSpawnGuardQuest = "spawn_guard";
    inline const QuestId ElderGuardInteractQuest = "guard_quest";
    inline const QuestId ElderVoidMistQuest = "void_mist";
    inline const QuestId ElderVoidMistExtraQuest = "void_mist_extra";

    // Побочные задания связанные с другими локациями или NPC заспавненными на них
    inline const QuestId JoeBackroadSideQuest = "joe_backroad_side_quest";
}

class QuestsSaveData {
    std::map<QuestId, QuestStatus> quests;
    QuestId currentActiveQuest; // Отмечаем исключительно текущий "СЮЖЕТНЫЙ" квест!

public:
    /// Текущий активный квест.
    const QuestId& GetCurrentActiveQuest() const { return currentActiveQuest; }
    QuestStatus GetStatus(const QuestId& id) const {
        if (auto it = quests.find(id); it != quests.end()) {
            return it->second;
        }
        return QuestStatus::NotStarted;
    }

    void SetStatus(const QuestId& id, QuestStatus status) {
        quests[id] = status;
    }

    void SetCurrentActiveQuest(const QuestId& id) {
        currentActiveQuest = id;
    }

    /// Состояние всех квестов в игре (текущее сохранение).
    [[nodiscard]] const std::map<QuestId, QuestStatus>& GetQuests() const { return quests; }

    void ToJson(nlohmann::json& j) const;
    void FromJson(const nlohmann::json& j);
    void ResetToDefaults();
};

inline void QuestsSaveData::ResetToDefaults() {
    quests.clear();

    // Инициализируем все известные квесты в состояние NotStarted.
    quests[World::VoidFirstEntranceQuest]  = QuestStatus::NotStarted;
    quests[World::JoeCarrotQuest]          = QuestStatus::NotStarted;
    quests[World::CowFeededQuest]          = QuestStatus::NotStarted;
    quests[World::ElderFirstMeetingQuest]  = QuestStatus::NotStarted;
    quests[World::ElderCatQuestInfo]       = QuestStatus::NotStarted;
    quests[World::ElderCatQuest]           = QuestStatus::NotStarted;
    quests[World::ElderSpawnGuardQuest]    = QuestStatus::NotStarted;
    quests[World::ElderGuardInteractQuest] = QuestStatus::NotStarted;
    quests[World::ElderVoidMistQuest]      = QuestStatus::NotStarted;
    quests[World::ElderVoidMistExtraQuest] = QuestStatus::NotStarted;
    quests[World::JoeBackroadSideQuest]    = QuestStatus::NotStarted;
    quests[World::JoeCarrotFinal]          = QuestStatus::NotStarted;

    currentActiveQuest = World::ElderFirstMeetingQuest;
}

inline void QuestsSaveData::ToJson(nlohmann::json& j) const {
    j = nlohmann::json::object();

    auto& qs = j["quests"];
    qs = nlohmann::json::object();
    for (const auto& [id, status] : quests) {
        qs[id] = static_cast<int>(status);
    }

    j["currentActiveQuest"] = currentActiveQuest;
}

inline void QuestsSaveData::FromJson(const nlohmann::json& j) {
    ResetToDefaults();

    if (!j.is_object()) {
        return;
    }

    if (j.contains("quests") && j["quests"].is_object()) {
        quests.clear();
        for (const auto& [id, value] : j["quests"].items()) {
            if (!value.is_number_integer()) {
                continue;
            }
            int raw = value.get<int>();

            QuestStatus status = QuestStatus::NotStarted;
            switch (raw) {
                case 1: status = QuestStatus::OnGoing;    break;
                case 2: status = QuestStatus::Completed;  break;
                default: status = QuestStatus::NotStarted; break;
            }

            quests[id] = status;
        }
    }

    if (j.contains("currentActiveQuest") && j["currentActiveQuest"].is_string()) {
        currentActiveQuest = j["currentActiveQuest"].get<std::string>();
    }

    // Гарантируем наличие всех известных квестов.
    auto ensureQuest = [this](const QuestId& id) {
        if (quests.find(id) == quests.end()) {
            quests[id] = QuestStatus::NotStarted;
        }
    };

    ensureQuest(World::VoidFirstEntranceQuest);
    ensureQuest(World::JoeCarrotQuest);
    ensureQuest(World::JoeCarrotFinal);
    ensureQuest(World::CowFeededQuest);
    ensureQuest(World::ElderFirstMeetingQuest);
    ensureQuest(World::ElderCatQuestInfo);
    ensureQuest(World::ElderCatQuest);
    ensureQuest(World::ElderSpawnGuardQuest);
    ensureQuest(World::ElderGuardInteractQuest);
    ensureQuest(World::ElderVoidMistQuest);
    ensureQuest(World::ElderVoidMistExtraQuest);
    ensureQuest(World::JoeBackroadSideQuest);
}
