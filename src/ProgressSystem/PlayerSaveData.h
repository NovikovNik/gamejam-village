#pragma once
#include "BasicSaveData.h"
#include <glm/glm.hpp>
#include <string>
#include <map>
#include "../Game/GameplayLogic.h"

namespace DialogTrackIds {
    inline const std::string Tutorial       = "tutorial";
    inline const std::string JoeCarrot      = "joe_carrot";
    inline const std::string JoeOldHouse    = "joe_old_house";
    inline const std::string JoeEldersHouse = "joe_elders_house";
    inline const std::string JoeAssembly    = "joe_assembly";
    inline const std::string GuardAssembly  = "guard_assembly";
    inline const std::string ElderAssembly  = "elder_assembly";
    inline const std::string CowAssemblyHall = "cow_assembly_hall";
    inline const std::string CatAssemblyHall = "cat_assembly_hall";
    inline const std::string Nebula         = "nebula";
}

struct PlayerSaveData : public BasicSaveData {
    glm::vec2 position{0.f, 0.f};
    std::string lastLevel = "assets/maps/intro.json";
    std::string lastGameAct = std::string(GameActIds::Intro);

    bool fileSystemIconVisible = false;
    bool gameEnded = false;
    bool catWasDestroyed = false;

    std::map<std::string, int> dialogProgress;

    int GetVersion() const override {
        return 1;
    }

    int GetDialogProgress(const std::string& id) const {
        if (auto it = dialogProgress.find(id); it != dialogProgress.end()) {
            return it->second;
        }
        return 0;
    }

    int AdvanceDialogProgress(const std::string& id, int maxValue) {
        int& value = dialogProgress[id];
        if (value < maxValue) {
            ++value;
            return value;
        }
        return maxValue;
    }

    void ResetToDefaults() override {
        position = {0.f, 0.f};
        lastLevel = "assets/maps/intro.json";
        lastGameAct = std::string(GameActIds::Intro);
        fileSystemIconVisible = false;
        gameEnded = false;
        catWasDestroyed = false;
        dialogProgress.clear();
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
        j["gameEnded"] = gameEnded;
        j["catWasDestroyed"] = catWasDestroyed;

        nlohmann::json dialogObj = nlohmann::json::object();
        for (const auto& [id, value] : dialogProgress) {
            dialogObj[id] = value;
        }
        j["dialogProgress"] = std::move(dialogObj);
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
        if (j.contains("gameEnded") && j["gameEnded"].is_boolean()) {
            gameEnded = j["gameEnded"].get<bool>();
        }
        if (j.contains("catWasDestroyed") && j["catWasDestroyed"].is_boolean()) {
            catWasDestroyed = j["catWasDestroyed"].get<bool>();
        }

        dialogProgress.clear();
        if (j.contains("dialogProgress") && j["dialogProgress"].is_object()) {
            for (const auto& [id, value] : j["dialogProgress"].items()) {
                if (value.is_number_integer()) {
                    dialogProgress[id] = value.get<int>();
                }
            }
        }

        if (!Validate()) {
            ResetToDefaults();
        }
    }

    [[maybe_unused]] bool Validate() const override {
        return !lastLevel.empty();
    }
};