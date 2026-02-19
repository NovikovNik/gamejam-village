#pragma once
#include <libs/json/single_include/nlohmann/json.hpp>

struct BasicSaveData {
    virtual ~BasicSaveData() = default;

    // Версия конкретной структуры
    virtual int GetVersion() const = 0;

    // Сериализация
    virtual void ToJson(nlohmann::json& out) const = 0;
    virtual void FromJson(const nlohmann::json& in) = 0;

    // Сброс к дефолтам
    [[maybe_unused]] virtual void ResetToDefaults() {}
    [[maybe_unused]] virtual bool Validate() const { return true; }
    [[maybe_unused]] virtual void Migrate(int fromVersion) {}
};