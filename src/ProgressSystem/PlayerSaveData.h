#pragma once
#include "BasicSaveData.h"
#include <glm/glm.hpp>
#include <string>

struct PlayerSaveData : public BasicSaveData {
    glm::vec2 position{0.f, 0.f};
    std::string lastLevel = "assets/maps/intro.json";

    int GetVersion() const override {
        return 1;
    }

    void ResetToDefaults() override {
        position = {0.f, 0.f};
        lastLevel = "assets/maps/intro.json";
    }

    void ToJson(nlohmann::json& j) const override {
        j["version"] = GetVersion();
        j["position"] = {
            {"x", position.x},
            {"y", position.y}
        };
        j["lastLevel"] = lastLevel;
    }

    void FromJson(const nlohmann::json& j) override {
        if (!j.is_object()) {
            ResetToDefaults();
            return;
        }
        int fileVersion = j.value("version", 0);

        if (fileVersion < GetVersion()) {
            Migrate(fileVersion);
        }

        if (j.contains("position") && j["position"].is_object()) {
            position.x = j["position"].value("x", 0.f);
            position.y = j["position"].value("y", 0.f);
        }

        if (j.contains("lastLevel") && j["lastLevel"].is_string()) {
            lastLevel = j["lastLevel"].get<std::string>();
        }

        if (!Validate()) {
            ResetToDefaults();
        }
    }

    [[maybe_unused]] bool Validate() const override {
        return !lastLevel.empty();
    }
};