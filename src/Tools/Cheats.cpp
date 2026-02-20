#include "Cheats.h"

#if ENABLE_CHEATS

#include <Renderer/Renderer.h>
#include <Renderer/Camera.h>
#include <imgui/imgui.h>
#include <Game/GameFeatures.h>
#include <Map/Map.h>
#include <Entities/EPlayer.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Utils/Singleton.h>
#include <DialogSystem/DialogSystem.h>
#include <EventBus/EventBus.h>
#include <Events/ForceDialogStartEvent.h>
#include <Game/GameplayLogic.h>
#include <Gameplay/WorldState.h>
#include <Gameplay/LocationsStates.h>
#include <string>
#include <algorithm>
#include <format>
#include <filesystem>
#include <vector>
#include <set>
#include <utility>

class CheatsManger : public Singleton<CheatsManger>
{
public:
    void UpdateAndRender()
    {
        if (!isCheatsActive) {
            return;
        }
        
        // Begin ImGui frame and render ImGui content
        Renderer::BeginImGuiFrame();
        
        // Cheats window
        ImGui::Begin("Cheats Menu", &isCheatsActive);
        ImGui::Text("Press ~ to close");
        ImGui::Separator();

        if (ImGui::BeginTabBar("CheatsTabs")) {
            if (ImGui::BeginTabItem("Main")) {
                ImGui::Separator();
                RefreshMapListIfNeeded();
        if (!mapNames.empty()) {
            const char* preview = selectedMapIndex >= 0 && selectedMapIndex < static_cast<int>(mapNames.size())
                ? mapNames[selectedMapIndex].c_str() : "Select level...";
            if (ImGui::BeginCombo("Level", preview)) {
                for (int i = 0; i < static_cast<int>(mapNames.size()); ++i) {
                    const bool isSelected = (selectedMapIndex == i);
                    if (ImGui::Selectable(mapNames[i].c_str(), isSelected)) {
                        selectedMapIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Load Map") && selectedMapIndex >= 0 && selectedMapIndex < static_cast<int>(mapNames.size())) {
                const auto fullPath = std::format("assets/maps/{}.json", mapNames[selectedMapIndex]);
                MapManager::LoadMap(fullPath);
            }
        } else {
            ImGui::Text("No levels found in assets/maps/");
        }

        ImGui::Separator();
        RefreshDialogListIfNeeded();
        if (!dialogOptions.empty()) {
            const char* dialogPreview = selectedDialogIndex >= 0 && selectedDialogIndex < static_cast<int>(dialogOptions.size())
                ? dialogOptions[selectedDialogIndex].c_str() : "Select dialog...";
            if (ImGui::BeginCombo("Dialog (Source:dialog_id)", dialogPreview)) {
                for (int i = 0; i < static_cast<int>(dialogOptions.size()); ++i) {
                    const bool isSelected = (selectedDialogIndex == i);
                    if (ImGui::Selectable(dialogOptions[i].c_str(), isSelected)) {
                        selectedDialogIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Start Dialog") && selectedDialogIndex >= 0 && selectedDialogIndex < static_cast<int>(dialogOptions.size())) {
                const auto& [characterId, dialogId] = dialogIds[selectedDialogIndex];
                EventBus::instance().EmitEvent<ForceDialogStartEvent>(characterId, dialogId);
            }
        } else {
            ImGui::Text("No dialogs loaded");
        }

        ImGui::Separator();
        const auto actIds = GameActIds::GetAllGameActIds();
        if (!actIds.empty()) {
            const std::string currentAct = GameplayLogic::GetCurrentGameActId();
            const char* actPreview = currentAct.c_str();
            if (ImGui::BeginCombo("Game Act", actPreview)) {
                for (size_t i = 0; i < actIds.size(); ++i) {
                    const std::string actStr(actIds[i]);
                    const bool isSelected = (currentAct == actStr);
                    if (ImGui::Selectable(actStr.c_str(), isSelected)) {
                        GameplayLogic::LoadGameAct(actIds[i]);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Select act to load (saves and applies immediately)");
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Reset Save")) {
            ProgressSystemManager::Player().ResetToDefaults();
            ProgressSystemManager::SaveData();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reset save data to defaults and overwrite gamesave.json");
        }
        
        // Debug info
        if (GameFeatures::isDebug) {
            ImGui::Separator();
            ImGui::Text("Debug Mode: ON");

            const auto& entities = MapManager::GetEntitiesContainer();
            if (auto* player = entities.FindEntity<World::EPlayer>()) {
                const auto pos = player->GetPosition();
                ImGui::Text("Player Pos: X=%.1f Y=%.1f", pos.x, pos.y);
            } else {
                ImGui::Text("Player: <not found>");
            }

            const auto cameraPos = World::Camera::instance().GetPosition();
            ImGui::Text("Camera Pos: X=%.1f Y=%.1f", cameraPos.x, cameraPos.y);
            ImGui::Text("Camera Scale: %.1f", World::Camera::instance().GetScaleFactor());
            if (ImGui::Button("Increase Camera Scale")) {
                World::Camera::instance().SetScaleFactor(World::Camera::instance().GetScaleFactor() + 0.1f);
            }
            if (ImGui::Button("Decrease Camera Scale")) {
                World::Camera::instance().SetScaleFactor(World::Camera::instance().GetScaleFactor() - 0.1f);
            }
        }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("World State")) {
                ImGui::Text("Objects by location (current vs last seen)");
                ImGui::Separator();
                const auto& current = WorldState::GetCurrentState();
                const auto& lastSeen = WorldState::GetLastSeenState();
                std::set<std::string> allLocations;
                for (const auto& [loc, _] : current) allLocations.insert(loc);
                for (const auto& [loc, _] : lastSeen) allLocations.insert(loc);
                for (const auto& locationName : allLocations) {
                    const auto& curObjs = current.count(locationName) ? current.at(locationName) : LocationsStates::LocationObjects{};
                    const auto& lastObjs = lastSeen.count(locationName) ? lastSeen.at(locationName) : LocationsStates::LocationObjects{};
                    LocationsStates::LocationObjects added, removed;
                    for (const auto& o : curObjs) {
                        if (std::find(lastObjs.begin(), lastObjs.end(), o) == lastObjs.end())
                            added.push_back(o);
                    }
                    for (const auto& o : lastObjs) {
                        if (std::find(curObjs.begin(), curObjs.end(), o) == curObjs.end())
                            removed.push_back(o);
                    }
                    const bool hasChanges = !added.empty() || !removed.empty();
                    const char* label = locationName.empty() ? "(root)" : locationName.c_str();
                    if (hasChanges) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.3f, 0.6f));
                    }
                    if (ImGui::TreeNode(label)) {
                        if (hasChanges) ImGui::PopStyleColor();
                        ImGui::Text("Current: %zu | Last seen: %zu", curObjs.size(), lastObjs.size());
                        if (!added.empty()) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.f));
                            ImGui::Text("Added (%zu):", added.size());
                            ImGui::PopStyleColor();
                            for (const auto& o : added)
                                ImGui::BulletText("%s.%s", o.name.c_str(), o.type.c_str());
                        }
                        if (!removed.empty()) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.2f, 1.f));
                            ImGui::Text("Removed (%zu):", removed.size());
                            ImGui::PopStyleColor();
                            for (const auto& o : removed)
                                ImGui::BulletText("%s.%s", o.name.c_str(), o.type.c_str());
                        }
                        if (added.empty() && removed.empty()) {
                            for (const auto& o : curObjs)
                                ImGui::BulletText("%s.%s", o.name.c_str(), o.type.c_str());
                        }
                        ImGui::TreePop();
                    } else if (hasChanges) {
                        ImGui::PopStyleColor();
                    }
                }
                if (allLocations.empty()) {
                    ImGui::Text("No location data (focus window to refresh from village/)");
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        
        ImGui::End();
        
        // End ImGui frame
        Renderer::EndImGuiFrame();
    }
    
    void ToggleCheats() {
        isCheatsActive = !isCheatsActive;
        mapListDirty = true;
        dialogListDirty = true;
    }

    void RefreshMapListIfNeeded() {
        if (!mapListDirty) return;
        mapListDirty = false;
        mapNames.clear();
        std::string mapsPath = "assets/maps";
        if (!std::filesystem::exists(mapsPath)) return;
        for (const auto& entry : std::filesystem::directory_iterator(mapsPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                mapNames.push_back(entry.path().stem().string());
            }
        }
        if (selectedMapIndex >= static_cast<int>(mapNames.size())) {
            selectedMapIndex = mapNames.empty() ? -1 : 0;
        }
    }

    void RefreshDialogListIfNeeded() {
        if (!dialogListDirty) return;
        dialogListDirty = false;
        dialogOptions.clear();
        dialogIds.clear();
        const auto& dialogs = DialogSystemManager::GetDialogs();
        for (const auto& [characterId, dialogsMap] : dialogs) {
            for (const auto& [dialogId, _] : dialogsMap) {
                dialogOptions.push_back(std::format("{}:{}", characterId, dialogId));
                dialogIds.emplace_back(characterId, dialogId);
            }
        }
        if (selectedDialogIndex >= static_cast<int>(dialogOptions.size())) {
            selectedDialogIndex = dialogOptions.empty() ? -1 : 0;
        }
    }
    
    bool AreCheatsActive() const {
        return isCheatsActive;
    }
    
private:
    std::vector<std::string> mapNames;
    int selectedMapIndex = -1;
    bool mapListDirty = true;

    std::vector<std::string> dialogOptions;
    std::vector<std::pair<std::string, std::string>> dialogIds;
    int selectedDialogIndex = -1;
    bool dialogListDirty = true;

    bool isCheatsActive = false;
};

void Cheats::UpdateAndRender() {
    CheatsManger::instance().UpdateAndRender();
}

void Cheats::ToggleCheats() {
    CheatsManger::instance().ToggleCheats();
}

bool Cheats::AreCheatsActive() {
    return CheatsManger::instance().AreCheatsActive();
}

#else

void Cheats::UpdateAndRender() {}
void Cheats::ToggleCheats() {}
bool Cheats::AreCheatsActive() { return false; }

#endif