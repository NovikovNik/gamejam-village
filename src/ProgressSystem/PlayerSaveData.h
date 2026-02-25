#pragma once
#include "BasicSaveData.h"
#include <glm/glm.hpp>
#include <string>
#include "../Game/GameplayLogic.h"

struct PlayerSaveData : public BasicSaveData {
    glm::vec2 position{0.f, 0.f};
    std::string lastLevel = "assets/maps/intro.json";
    std::string lastGameAct = std::string(GameActIds::Intro);
    bool fileSystemIconVisible = false;
    int tutorialDialogProgress = 0;

    int GetVersion() const override {
        return 1;
    }

    void ResetToDefaults() override {
        position = {0.f, 0.f};
        lastLevel = "assets/maps/intro.json";
        lastGameAct = std::string(GameActIds::Intro);
        fileSystemIconVisible = false;
        tutorialDialogProgress = 0;
    }

    void ToJson(nlohmann::json& j) const override {
        j["version"] = GetVersion();
        j["position"] = {
            {"x", position.x},
            {"y", position.y}
        };
        j["lastLevel"] = lastLevel;
        j["lastGameAct"] = lastGameAct;
        j["fileSystemIconVisible"] = fileSystemIconVisible;
        j["tutorialDialogProgress"] = tutorialDialogProgress;
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

        if (j.contains("lastGameAct") && j["lastGameAct"].is_string()) {
            lastGameAct = j["lastGameAct"].get<std::string>();
        }
        if (j.contains("fileSystemIconVisible") && j["fileSystemIconVisible"].is_boolean()) {
            fileSystemIconVisible = j["fileSystemIconVisible"].get<bool>();
        }
        if (j.contains("tutorialDialogProgress") && j["tutorialDialogProgress"].is_number_integer()) {
            tutorialDialogProgress = j["tutorialDialogProgress"].get<int>();
        }

        if (!Validate()) {
            ResetToDefaults();
        }
    }

    [[maybe_unused]] bool Validate() const override {
        return !lastLevel.empty();
    }
};