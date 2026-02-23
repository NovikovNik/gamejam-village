#include "Cheats.h"

#if ENABLE_CHEATS

#include <Renderer/Renderer.h>
#include <Renderer/Camera.h>
#include <Renderer/UISystem.h>
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
#include <sstream>
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

        ImGui::Separator();
        if (ImGui::Button("Show focus hint")) {
            UISystem::FocusHint();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show filesystem icon focus hint animation");
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

            if (ImGui::BeginTabItem("Sign")) {
                ImGui::Text("Each line becomes one row on the sign.");
                ImGui::Separator();
                ImGui::InputTextMultiline("##sign", signBuf, sizeof(signBuf),
                    ImVec2(-1.f, 120.f));
                if (ImGui::Button("Open Sign")) {
                    std::vector<std::string> rows;
                    std::istringstream ss(signBuf);
                    std::string line;
                    while (std::getline(ss, line)) {
                        rows.push_back(line);
                    }
                    DialogSystemManager::OpenSign(rows);
                }
                ImGui::SameLine();
                if (ImGui::Button("Close Sign")) {
                    DialogSystemManager::CloseSign();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("World State")) {
                const auto& state = WorldState::GetCurrentState();
                if (state.registeredEntities.empty()) {
                    ImGui::TextDisabled("(no entities registered)");
                } else {
                    ImGui::Text("Registered Entities");
                    ImGui::Separator();
                    for (const auto& [entityKey, locations] : state.registeredEntities) {
                        const char* label = entityKey.empty() ? "(root)" : entityKey.c_str();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.4f, 1.f));
                        ImGui::Text("%s (%zu)", label, locations.size());
                        ImGui::PopStyleColor();
                        for (const auto& location : locations) {
                            ImGui::BulletText("%s", location.c_str());
                        }
                    }
                    ImGui::Separator();
                    ImGui::Text("Active Locations");
                    ImGui::Separator();
                    for (const auto& location : state.registeredLocations) {
                        ImGui::BulletText("%s", location.first.c_str());
                        if (location.second) {
                            ImGui::Text("Active");
                        } else {
                            ImGui::Text("Not active");
                        }
                    }
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
    char signBuf[1024] = "Line one\nLine two\nLine three\nLine four\nLine five\nLine six\nLine seven\nLine eight";
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