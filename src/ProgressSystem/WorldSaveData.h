#pragma once
#include "BasicSaveData.h"
#include <Gameplay/LocationsStates.h>
#include <set>
#include <string>

struct WorldSaveData : public BasicSaveData {
    LocationsStates::State state;
    std::string backupLocationPath;
    std::set<std::string> removedEntities;
    std::set<std::string> visitedLocations;

    int GetVersion() const override { return 1; }

    void ResetToDefaults() override {
        state = {};
        backupLocationPath = "";
        removedEntities.clear();
        visitedLocations.clear();
    }

    void ToJson(nlohmann::json& j) const override {
        j["version"] = GetVersion();

        auto& locs = j["registeredLocations"];
        locs = nlohmann::json::object();
        for (const auto& [loc, active] : state.registeredLocations) {
            locs[loc] = active;
        }

        auto& ents = j["registeredEntities"];
        ents = nlohmann::json::object();
        for (const auto& [key, locations] : state.registeredEntities) {
            auto arr = nlohmann::json::array();
            for (const auto& loc : locations) {
                arr.push_back(loc);
            }
            ents[key] = std::move(arr);
        }
        j["backupLocationPath"] = backupLocationPath;
        j["removedEntities"] = removedEntities;
        j["visitedLocations"] = visitedLocations;
    }

    void FromJson(const nlohmann::json& j) override {
        if (!j.is_object()) { ResetToDefaults(); return; }

        state = {};
        removedEntities.clear();
        visitedLocations.clear();

        if (j.contains("registeredLocations") && j["registeredLocations"].is_object()) {
            for (const auto& [loc, active] : j["registeredLocations"].items()) {
                state.registeredLocations[loc] = active.get<bool>();
            }
        }

        if (j.contains("registeredEntities") && j["registeredEntities"].is_object()) {
            for (const auto& [key, arr] : j["registeredEntities"].items()) {
                std::set<std::string> locations;
                if (arr.is_array()) {
                    for (const auto& loc : arr) {
                        locations.insert(loc.get<std::string>());
                    }
                }
                state.registeredEntities[key] = std::move(locations);
            }
        }
        if (j.contains("backupLocationPath") && j["backupLocationPath"].is_string()) {
            backupLocationPath = j["backupLocationPath"].get<std::string>();
        }
        if (j.contains("removedEntities") && j["removedEntities"].is_array()) {
            for (const auto& el : j["removedEntities"]) {
                removedEntities.insert(el.get<std::string>());
            }
        }
        if (j.contains("visitedLocations") && j["visitedLocations"].is_array()) {
            for (const auto& el : j["visitedLocations"]) {
                visitedLocations.insert(el.get<std::string>());
            }
        }
    }
};
