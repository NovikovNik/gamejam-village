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
    int joeCarrotQuestProgress = 0;
    int joeOldHouseQuestProgress = 0;
    int joeEldersHouseQuestProgress = 0;
    int joeAssemblyHallQuestProgress = 0;
    int guardAssemblyHallQuestProgress = 0;
    int nebulaQuestProgress = 0;

    int GetVersion() const override {
        return 1;
    }

    void ResetToDefaults() override {
        position = {0.f, 0.f};
        lastLevel = "assets/maps/intro.json";
        lastGameAct = std::string(GameActIds::Intro);
        fileSystemIconVisible = false;
        tutorialDialogProgress = 0;
        joeCarrotQuestProgress = 0;
        joeOldHouseQuestProgress = 0;
        joeEldersHouseQuestProgress = 0;
        guardAssemblyHallQuestProgress = 0;
        joeAssemblyHallQuestProgress = 0;
        nebulaQuestProgress = 0;
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
        j["joeCarrotQuestProgress"] = joeCarrotQuestProgress;
        j["joeOldHouseQuestProgress"] = joeOldHouseQuestProgress;
        j["joeEldersHouseQuestProgress"] = joeEldersHouseQuestProgress;
        j["guardAssemblyHallQuestProgress"] = guardAssemblyHallQuestProgress;
        j["joeAssemblyHallQuestProgress"] = joeAssemblyHallQuestProgress;
        j["nebulaQuestProgress"] = nebulaQuestProgress;
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
        if (j.contains("joeCarrotQuestProgress") && j["joeCarrotQuestProgress"].is_number_integer()) {
            joeCarrotQuestProgress = j["joeCarrotQuestProgress"].get<int>();
        }
        if (j.contains("joeOldHouseQuestProgress") && j["joeOldHouseQuestProgress"].is_number_integer()) {
            joeOldHouseQuestProgress = j["joeOldHouseQuestProgress"].get<int>();
        }
        if (j.contains("joeEldersHouseQuestProgress") && j["joeEldersHouseQuestProgress"].is_number_integer()) {
            joeEldersHouseQuestProgress = j["joeEldersHouseQuestProgress"].get<int>();
        }
        if (j.contains("guardAssemblyHallQuestProgress") && j["guardAssemblyHallQuestProgress"].is_number_integer()) {
            guardAssemblyHallQuestProgress = j["guardAssemblyHallQuestProgress"].get<int>();
        }
        if (j.contains("joeAssemblyHallQuestProgress") && j["joeAssemblyHallQuestProgress"].is_number_integer()) {
            joeAssemblyHallQuestProgress = j["joeAssemblyHallQuestProgress"].get<int>();
        }
        if (j.contains("nebulaQuestProgress") && j["nebulaQuestProgress"].is_number_integer()) {
            nebulaQuestProgress = j["nebulaQuestProgress"].get<int>();
        }

        if (!Validate()) {
            ResetToDefaults();
        }
    }

    [[maybe_unused]] bool Validate() const override {
        return !lastLevel.empty();
    }
};