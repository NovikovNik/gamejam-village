#include "Renderer.h"
#include <Utils/Singleton.h>
#include "../Logger/Logger.h"
#include "../Game/GameFeatures.h"
#include "Renderer/Camera.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_rect.h"
#include <glm/glm.hpp>

#include "../Events/WindowResizedEvent.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <map>
#include <string>
#include <cstdint>
#include <set>
#include <filesystem>

class RenderManager : public Singleton<RenderManager> {
public:
    void Initialize(int32_t windowWidth, int32_t windowHeight)
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            Logger::Err("SDL_Init failed: " + std::string(SDL_GetError()));
            return;
        }

        if (!TTF_Init()) {
            Logger::Err("TTF_Init failed: " + std::string(SDL_GetError()));
            return;
        }

        Uint32 windowFlags = GameFeatures::isResizeble ? SDL_WINDOW_RESIZABLE : 0;
        if (!SDL_CreateWindowAndRenderer(GameFeatures::windowTitle.c_str(), windowWidth, windowHeight, windowFlags, &window, &renderer)) {
            SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
            return;
        }
        SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_DISABLED);
        UpdateWindowOutputSize();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
        eventBus->SubscribeToEvent<WindowResizedEvent>(this, &RenderManager::OnWindowSizeChanged);
    }

    void OnWindowSizeChanged(WindowResizedEvent&) {
        UpdateWindowOutputSize();
    }

    void UpdateWindowOutputSize() {
        SDL_GetCurrentRenderOutputSize(renderer, &renderOutputSizeW, &renderOutputSizeH);
        Logger::Log(std::to_string(renderOutputSizeW) + " | " + std::to_string(renderOutputSizeH));
    }

    void LoadTexture(const std::filesystem::path& texturePath) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, texturePath.string().c_str());
        if (!texture) {
            SDL_Log("Texture creation failed: %s", SDL_GetError());
        }

        textures.insert({ make_nnTex(texturePath.filename().stem().string()), texture });
    }

    void UnloadTextures() {
        for (const auto& texture : textures) {
            SDL_DestroyTexture(texture.second);
        }
        textures.clear();
    }

    void LoadAllTextures(const std::string& directory) {
        UnloadTextures();
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".png") {
               LoadTexture(entry.path());
            }
        }
    }

    void DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height) {
        SDL_Texture* texture = textures[textureId];
        if (texture == nullptr) {
            return;
        }
        SDL_FRect rect;
        glm::vec2 cameraPos = World::Camera::instance().GetPosition();

        rect.x = renderOutputSizeW * 0.5f + (x - cameraPos.x) - width * 0.5f;
        rect.y = renderOutputSizeH * 0.5f + (y - cameraPos.y) - height * 0.5f;
        rect.w = width;
        rect.h = height;

        SDL_RenderTexture(renderer, texture, NULL, &rect);
    }

    void DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height, double angle) {
        SDL_Texture* texture = textures[textureId];
        if (texture == nullptr) {
            return;
        }

        SDL_FRect rect;
        glm::vec2 cameraPos = World::Camera::instance().GetPosition();

        rect.x = renderOutputSizeW * 0.5f + (x - cameraPos.x) - width * 0.5f;
        rect.y = renderOutputSizeH * 0.5f + (y - cameraPos.y) - height * 0.5f;
        rect.w = width;
        rect.h = height;

        SDL_FPoint center = { width / 2.0f, height / 2.0f };
        SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);
    }

    void DrawRectangle(float x, float y, float w, float h, float angle) {
        glm::vec2 cam = World::Camera::instance().GetPosition();

        glm::vec2 center(
            renderOutputSizeW * 0.5f + (x - cam.x),
            renderOutputSizeH * 0.5f + (y - cam.y)
        );

        float rad = glm::radians(angle);
        float c = cos(rad);
        float s = sin(rad);

        glm::vec2 corners[4] = {
            {-w * 0.5f, -h * 0.5f},
            {  w * 0.5f, -h * 0.5f},
            {  w * 0.5f,  h * 0.5f},
            { -w * 0.5f,  h * 0.5f}
        };

        SDL_FPoint pts[5];

        for (int i = 0; i < 4; ++i) {
            glm::vec2 p(
                corners[i].x * c - corners[i].y * s,
                corners[i].x * s + corners[i].y * c
            );

            p += center;

            pts[i] = { p.x, p.y };
        }

        pts[4] = pts[0];

        SDL_SetRenderDrawColor(renderer, 244, 0, 180, 255);
        SDL_RenderLines(renderer, pts, 5);
    }

    void BeginRender() {
        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_RenderClear(renderer);
    }

    void EndRender() {
        SDL_RenderPresent(renderer);
    }

    void SetCameraPosition(float x, float y) {
    }

    void PrintFPSinTitle(const float& fps_live) {
        char title[64];
        std::snprintf(title, sizeof(title), "%s - FPS: %.1f", GameFeatures::windowTitle.c_str(), fps_live);
        SDL_SetWindowTitle(window, title);
    }

    void Destroy() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Renderer* renderer {};
    SDL_Window* window {};
    std::map<Renderer::TextureId, SDL_Texture*> textures;

    int renderOutputSizeW, renderOutputSizeH;

};

void Renderer::Initialize(int32_t windowWidth, int32_t windowHeight) {
    RenderManager::instance().Initialize(windowWidth, windowHeight);
}

void Renderer::Destroy() {
    RenderManager::instance().Destroy();
}

void Renderer::SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
    RenderManager::instance().SubscribeToEvents(eventBus);
}

void Renderer::DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height) {
    RenderManager::instance().DrawSprite(textureId, x, y, width, height);
}

void Renderer::DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height, double angle) {
    RenderManager::instance().DrawSprite(textureId, x, y, width, height, angle);
}

void Renderer::DrawRectangle(float x, float y, float w, float h, float angle = 0.0) {
    RenderManager::instance().DrawRectangle(x, y, w, h, angle);
}

void Renderer::SetCameraPosition(float x, float y) {
    RenderManager::instance().SetCameraPosition(x, y);
}

void Renderer::LoadAllTextures(const std::string& directory) {
    RenderManager::instance().LoadAllTextures(directory);
}

void Renderer::BeginRender() {
    RenderManager::instance().BeginRender();
}

void Renderer::EndRender() {
    RenderManager::instance().EndRender();
}

void Renderer::PrintFPSinTitle(const float& fps_live) {
    RenderManager::instance().PrintFPSinTitle(fps_live);
}