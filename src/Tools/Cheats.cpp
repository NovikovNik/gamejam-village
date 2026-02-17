#include "Cheats.h"

#include <Renderer/Renderer.h>
#include <imgui/imgui.h>
#include <Game/GameFeatures.h>
#include <Map/Map.h>
#include <utils/Singleton.h>
#include <string>
#include <format>

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
        
        ImGui::InputText("Map Name", mapName, sizeof(mapName));
        if (ImGui::Button("Load Map")) {
            const auto fullPath = std::format("assets/maps/{}.json", mapName);
            MapManager::LoadMap(fullPath);
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
    }
    
    bool AreCheatsActive() const {
        return isCheatsActive;
    }
    
private:
    char mapName[64] = {};
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
