#pragma once

#include <string>
#include <set>
#include <libs/json/single_include/nlohmann/json.hpp>

using ItemId = std::string;

namespace World {
    inline const ItemId message = "message";
    inline const ItemId sword = "sword";
    inline const ItemId key = "key";
    inline const ItemId carrot = "carrot";
    inline const ItemId book = "book";

    inline const std::set<ItemId> GetAllItems() { return {message, sword, key, carrot, book}; }
}

class InventarySaveData {
    std::set<ItemId> items;

public:
    /// Есть ли предмет в инвентаре.
    [[nodiscard]] bool HasItem(const ItemId& itemId) const;

    /// Добавить предмет (один экземпляр; повторный вызов с тем же id не дублирует).
    void AddItem(const ItemId& itemId);

    /// Удалить предмет из инвентаря.
    void RemoveItem(const ItemId& itemId);

    /// Список всех предметов в инвентаре (текущее сохранение).
    [[nodiscard]] const std::set<ItemId>& GetItems() const { return items; }

    void ToJson(nlohmann::json& j) const;
    void FromJson(const nlohmann::json& j);
    void ResetToDefaults();
};
