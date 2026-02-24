#include "InventarySaveData.h"

[[nodiscard]] bool InventarySaveData::HasItem(const ItemId& itemId) const {
    return items.count(itemId) != 0;
}

void InventarySaveData::AddItem(const ItemId& itemId) {
    items.insert(itemId);
}

void InventarySaveData::RemoveItem(const ItemId& itemId) {
    items.erase(itemId);
}

bool InventarySaveData::ChestBoxWasOpened(const ChestBoxId& chestBoxId) const {
    return chestBoxes.count(chestBoxId) != 0 && chestBoxes.at(chestBoxId) == ChestBoxStatus::Opened;
}

void InventarySaveData::SetChestBoxOpened(const ChestBoxId& chestBoxId) {
    chestBoxes[chestBoxId] = ChestBoxStatus::Opened;
}

void InventarySaveData::ToJson(nlohmann::json& j) const {
    j["items"] = nlohmann::json::array();
    for (const auto& id : items) {
        j["items"].push_back(id);
    }
    j["chestBoxes"] = nlohmann::json::object();
    for (const auto& [id, status] : chestBoxes) {
        j["chestBoxes"][id] = status;
    }
}

void InventarySaveData::FromJson(const nlohmann::json& j) {
    items.clear();
    if (!j.is_object() || !j.contains("items") || !j["items"].is_array()) {
        return;
    }
    for (const auto& el : j["items"]) {
        if (el.is_string()) {
            items.insert(el.get<std::string>());
        }
    }
    chestBoxes.clear();
    if (!j.is_object() || !j.contains("chestBoxes") || !j["chestBoxes"].is_object()) {
        return;
    }
    for (auto it = j["chestBoxes"].begin(); it != j["chestBoxes"].end(); ++it) {
        const std::string& id = it.key();
        const auto& statusJson = it.value();
        chestBoxes[id] = statusJson.get<ChestBoxStatus>();
    }
}

void InventarySaveData::ResetToDefaults() {
    items.clear();
    chestBoxes.clear();
    chestBoxes[World::chestBoxElderHouse] = ChestBoxStatus::NotOpened;
    chestBoxes[World::chestBoxOldHouse] = ChestBoxStatus::NotOpened;
    chestBoxes[World::swordBox] = ChestBoxStatus::NotOpened;
}
