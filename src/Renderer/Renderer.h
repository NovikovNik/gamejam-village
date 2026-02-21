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
    void DrawSprite(TextureId textureId, float x, float y, float width, float height, bool horizontalFlip = false, bool tiled = false);
    void DrawSprite(TextureId textureId, float x, float y, float width, float height, double angle, bool horizontalFlip = false);
    void DrawSprite(TextureId textureId, float x, float y, float width, float height, double angle);
    /// Спрайт в экранных координатах (без учёта камеры) — для UI
    void DrawSpriteScreen(TextureId textureId, float screenX, float screenY, float width, float height);
    void DrawSpriteScreen(TextureId textureId, float screenX, float screenY, float width, float height, double angle);
    // Debug render
    void DrawRectangle(float x, float y, float w, float h, float angle);
    void DrawCircle(float x, float y, float radius);
    /// Draw an axis-aligned filled rectangle in screen-space (UI / overlay).
    void DrawFilledRectScreen(float x, float y, float w, float h, SDL_Color color);
    void SetCameraPosition(float x, float y);
    void LoadAllTextures(const std::string& directory);
    void LoadAllFonts(const std::string& directory);
    /// Проверяет, загружена ли текстура (существует ли анимация/спрайт с таким id).
    [[nodiscard]] bool HasTexture(const TextureId textureId);
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

    struct AnimationHandle{
        int numOfFrames{};
        int maxElementsPerRow{};
        int frameSize{};
        float frameDelay{};
        int currentFrameId{};
        float currentFrameTime{};

        TextureId textureId{};
    };

    bool RenderAnimation(AnimationHandle& animationHandle, float deltaTime, float x, float y, float width, float height, bool horizontalFlip = false);
}
