#include "Cheats.h"

#include <Renderer/Renderer.h>
#include <imgui/imgui.h>
#include <Game/GameFeatures.h>
#include <Map/Map.h>
#include <Utils/Singleton.h>
#include <string>
#include <format>
#include <filesystem>
#include <vector>

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
    
    bool AreCheatsActive() const {
        return isCheatsActive;
    }
    
private:
    std::vector<std::string> mapNames;
    int selectedMapIndex = -1;
    bool mapListDirty = true;
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
