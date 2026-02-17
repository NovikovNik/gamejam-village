#pragma once
#include <string>
#include "../EventBus/EventBus.h"

#include <libs/CPPNanoString/includes/CPPNanoString.h>

nnstrINIT_TABLES(_nnTex, std::mutex, mt, std::vector, textureNamesRn, 64, std::array, textureNamesCt)

namespace Renderer {
    using TextureId = nnstr::NanoString;

    void Initialize(int32_t windowWidth, int32_t windowHeight);
    void Destroy();
    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus);
    void DrawSprite(TextureId textureId, float x, float y, float width, float height);
    void DrawSprite(TextureId textureId, float x, float y, float width, float height, double angle);
    // Debug render
    void DrawRectangle(float x, float y, float w, float h, float angle);
    void SetCameraPosition(float x, float y);
    void LoadAllTextures(const std::string& directory);

    void BeginRender();
    void EndRender();

    void PrintFPSinTitle(const float& fps_live);
}
