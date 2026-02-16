#pragma once
#include <string>

#include <libs/CPPNanoString/includes/CPPNanoString.h>

nnstrINIT_TABLES(_nnTex, std::mutex, mt, std::vector, textureNamesRn, 64, std::array, textureNamesCt)

namespace Renderer {
    using TextureId = nnstr::NanoString;

    void Initialize(int32_t windowWidth, int32_t windowHeight);
    void Destroy();
    void DrawSprite(TextureId textureId, float x, float y, float width, float height);
    void SetCameraPosition(float x, float y);
    void LoadAllTextures(const std::string& directory);

    void BeginRender();
    void EndRender();
}
