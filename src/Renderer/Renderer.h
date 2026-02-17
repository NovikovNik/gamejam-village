#pragma once
#include <string>
#include <EventBus/EventBus.h>

#include <libs/CPPNanoString/includes/CPPNanoString.h>

struct SDL_Color;
union SDL_Event;

nnstrINIT_TABLES(_nnTex, std::mutex, mt, std::vector, textureNamesRn, 64, std::array, textureNamesCt)

namespace Renderer {
    using TextureId = nnstr::NanoString;
    using TextId = nnstr::NanoString;

    void Initialize(int32_t windowWidth, int32_t windowHeight);
    void Destroy();
    void SubscribeToEvents();
    void DrawSprite(TextureId textureId, float x, float y, float width, float height);
    void DrawSprite(TextureId textureId, float x, float y, float width, float height, double angle);
    // Debug render
    void DrawRectangle(float x, float y, float w, float h, float angle);
    void SetCameraPosition(float x, float y);
    void LoadAllTextures(const std::string& directory);
    void LoadAllFonts(const std::string& directory);
    /// maxLineWidth: 0 = без переноса; >0 = макс. ширина строки в пикселях (перенос по словам)
    void DrawText(TextId fontId, const std::string& text, float x, float y, int fontSize = 16, const SDL_Color* color = nullptr, int maxLineWidth = 0);
    /// Текст в экранных координатах (без учёта камеры) — для UI. maxLineWidth: 0 = без переноса; >0 = перенос по словам
    void DrawTextScreen(TextId fontId, const std::string& text, float screenX, float screenY, int fontSize = 16, const SDL_Color* color = nullptr, int maxLineWidth = 0);

    void BeginRender();
    void EndRender();

    void PrintFPSinTitle(const float& fps_live);
    
    void ProcessImGuiEvent(SDL_Event* event);
    void BeginImGuiFrame();
    void EndImGuiFrame();
}
