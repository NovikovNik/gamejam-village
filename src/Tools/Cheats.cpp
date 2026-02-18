#include "Cheats.h"

#if ENABLE_CHEATS

#include <Renderer/Renderer.h>
#include <imgui/imgui.h>
#include <Game/GameFeatures.h>
#include <Map/Map.h>
#include <Utils/Singleton.h>
#include <DialogSystem/DialogSystem.h>
#include <EventBus/EventsQueue.h>
#include <Events/ForceDialogStartEvent.h>
#include <string>
#include <format>
#include <filesystem>
#include <vector>
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
                EventsQueue::instance().Push(ForceDialogStartEvent(characterId, dialogId));
            }
        } else {
            ImGui::Text("No dialogs loaded");
        }
        
        // Debug info
        if (GameFeatures::isDebug) {
            ImGui::Separator();
            ImGui::Text("Debug Mode: ON");
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