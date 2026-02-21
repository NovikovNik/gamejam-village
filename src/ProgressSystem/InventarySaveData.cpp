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

void InventarySaveData::ToJson(nlohmann::json& j) const {
    j["items"] = nlohmann::json::array();
    for (const auto& id : items) {
        j["items"].push_back(id);
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
}

void InventarySaveData::ResetToDefaults() {
    items.clear();
}
