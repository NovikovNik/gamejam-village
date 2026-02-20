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

// ImGui includes - SDL3 backends
#if ENABLE_CHEATS
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>
#endif

#include <map>
#include <string>
#include <cstdint>
#include <set>
#include <filesystem>
#include <utility>

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
        if (!SDL_CreateWindowAndRenderer(GameFeatures::gameTitle.c_str(), windowWidth, windowHeight, windowFlags, &window, &renderer)) {
            SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
            return;
        }
        SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_DISABLED);
        UpdateWindowOutputSize();

        SubscribeToEvents();
        InitializeImGui();
    }

    void SubscribeToEvents() {
        onWindowResized = EventBus::instance().SubscribeToEvent<WindowResizedEvent>(this, &RenderManager::OnWindowSizeChanged);
    }

    void OnWindowSizeChanged(WindowResizedEvent&) {
        UpdateWindowOutputSize();
    }

    void UpdateWindowOutputSize() {
        SDL_GetCurrentRenderOutputSize(renderer, &renderOutputSizeW, &renderOutputSizeH);
        Logger::Log("[Renderer] Window output size: " + std::to_string(renderOutputSizeW) + " | " + std::to_string(renderOutputSizeH));
    }

    void LoadTexture(const std::filesystem::path& texturePath) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, texturePath.string().c_str());
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
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

    void LoadFont(const std::filesystem::path& fontPath) {
        std::string path = fontPath.string();
        std::string id = fontPath.filename().stem().string();
        fontPaths.insert({ make_nnTex(id), path });
    }

    void UnloadFonts() {
        for (const auto& [key, font] : fontCache) {
            TTF_CloseFont(font);
        }
        fontCache.clear();
        fontPaths.clear();
    }

    void LoadAllFonts(const std::string& directory) {
        UnloadFonts();
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ttf") {
                LoadFont(entry.path());
            }
        }
    }

    TTF_Font* GetOrCreateFont(Renderer::TextId fontId, int ptsize) {
        auto it = fontPaths.find(fontId);
        if (it == fontPaths.end()) return nullptr;

        auto key = std::make_pair(it->second, ptsize);
        auto cached = fontCache.find(key);
        if (cached != fontCache.end()) return cached->second;

        TTF_Font* font = TTF_OpenFont(it->second.c_str(), static_cast<float>(ptsize));
        if (!font) return nullptr;
        fontCache[key] = font;
        return font;
    }

    void DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height) {
        SDL_Texture* texture = textures[textureId];
        if (texture == nullptr) {
            return;
        }
        SDL_FRect rect;
        glm::vec2 cameraPos = World::Camera::instance().GetPosition();
        float scaleFactor = World::Camera::instance().GetScaleFactor();

        // Единый масштаб: 1 world unit = scaleFactor pixels (позиция и размер)
        rect.x = renderOutputSizeW * 0.5f + (x - cameraPos.x) * scaleFactor - width * 0.5f * scaleFactor;
        rect.y = renderOutputSizeH * 0.5f + (y - cameraPos.y) * scaleFactor - height * 0.5f * scaleFactor;
        rect.w = width * scaleFactor;
        rect.h = height * scaleFactor;

        SDL_RenderTexture(renderer, texture, NULL, &rect);
    }

    void DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height, double angle) {
        SDL_Texture* texture = textures[textureId];
        if (texture == nullptr) {
            return;
        }

        SDL_FRect rect;
        glm::vec2 cameraPos = World::Camera::instance().GetPosition();
        float scaleFactor = World::Camera::instance().GetScaleFactor();

        // Единый масштаб: 1 world unit = scaleFactor pixels (позиция и размер)
        rect.x = renderOutputSizeW * 0.5f + (x - cameraPos.x) * scaleFactor - width * 0.5f * scaleFactor;
        rect.y = renderOutputSizeH * 0.5f + (y - cameraPos.y) * scaleFactor - height * 0.5f * scaleFactor;
        rect.w = width * scaleFactor;
        rect.h = height * scaleFactor;

        SDL_FPoint center = { width / 2.0f, height / 2.0f };
        SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);
    }

    void DrawSpriteScreen(Renderer::TextureId textureId, float screenX, float screenY, float width, float height) {
        DrawSpriteScreen(textureId, screenX, screenY, width, height, 0.0);
    }

    void DrawSpriteScreen(Renderer::TextureId textureId, float screenX, float screenY, float width, float height, double angle) {
        SDL_Texture* texture = textures[textureId];
        if (texture == nullptr) {
            return;
        }

        SDL_FRect rect;
        rect.x = screenX;
        rect.y = screenY;
        rect.w = width;
        rect.h = height;

        SDL_FPoint center = { width / 2.0f, height / 2.0f };
        SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);
    }

    void DrawRectangle(float x, float y, float w, float h, float angle) {
        glm::vec2 cam = World::Camera::instance().GetPosition();
        float scaleFactor = World::Camera::instance().GetScaleFactor();

        // Единый масштаб с DrawSprite: 1 world unit = scaleFactor pixels (позиция и размер)
        glm::vec2 center(
            renderOutputSizeW * 0.5f + (x - cam.x) * scaleFactor,
            renderOutputSizeH * 0.5f + (y - cam.y) * scaleFactor
        );

        const float hw = w * 0.5f * scaleFactor;
        const float hh = h * 0.5f * scaleFactor;

        float rad = glm::radians(angle);
        float c = cos(rad);
        float s = sin(rad);

        glm::vec2 corners[4] = {
            {-hw, -hh},
            { hw, -hh},
            { hw,  hh},
            {-hw,  hh}
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

    void DrawText(Renderer::TextId fontId, const std::string& text, float x, float y, int fontSize, const SDL_Color* color, int maxLineWidth) {
        TTF_Font* font = GetOrCreateFont(fontId, fontSize);
        if (!font || text.empty()) return;

        SDL_Color fg = color ? *color : SDL_Color{255, 255, 255, 255};
        SDL_Surface* surface = maxLineWidth > 0
            ? TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, fg, maxLineWidth)
            : TTF_RenderText_Blended(font, text.c_str(), 0, fg);
        if (!surface) return;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int w = surface->w, h = surface->h;
        SDL_DestroySurface(surface);
        if (!texture) return;

        glm::vec2 cameraPos = World::Camera::instance().GetPosition();
        float scaleFactor = World::Camera::instance().GetScaleFactor();
        SDL_FRect rect;
        rect.x = renderOutputSizeW * 0.5f + (x - cameraPos.x) * scaleFactor;
        rect.y = renderOutputSizeH * 0.5f + (y - cameraPos.y) * scaleFactor;
        rect.w = static_cast<float>(w);
        rect.h = static_cast<float>(h);

        SDL_RenderTexture(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }

    void DrawTextScreen(Renderer::TextId fontId, const std::string& text, float screenX, float screenY, int fontSize, const SDL_Color* color, int maxLineWidth) {
        TTF_Font* font = GetOrCreateFont(fontId, fontSize);
        if (!font || text.empty()) return;

        SDL_Color fg = color ? *color : SDL_Color{255, 255, 255, 255};
        SDL_Surface* surface = maxLineWidth > 0
            ? TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, fg, maxLineWidth)
            : TTF_RenderText_Blended(font, text.c_str(), 0, fg);
        if (!surface) return;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int w = surface->w, h = surface->h;
        SDL_DestroySurface(surface);
        if (!texture) return;

        SDL_FRect rect;
        rect.x = screenX;
        rect.y = screenY;
        rect.w = static_cast<float>(w);
        rect.h = static_cast<float>(h);

        SDL_RenderTexture(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
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
        std::snprintf(title, sizeof(title), "%s - FPS: %.1f", GameFeatures::gameTitle.c_str(), fps_live);
        SDL_SetWindowTitle(window, title);
    }

    void InitializeImGui() {
#if ENABLE_CHEATS
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Initialize ImGui SDL3 backends
        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);
        
        Logger::Log("[Renderer] ImGui initialized");
#endif
    }
    
    void ShutdownImGui() {
#if ENABLE_CHEATS
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        
        Logger::Log("[Renderer] ImGui shut down");
#endif
    }
    
    void ProcessImGuiEvent(SDL_Event* event) {
#if ENABLE_CHEATS
        ImGui_ImplSDL3_ProcessEvent(event);
#endif
    }
    
    void BeginImGuiFrame() {
#if ENABLE_CHEATS
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
#endif
    }
    
    void EndImGuiFrame() {
#if ENABLE_CHEATS
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
#endif
    }

    void Destroy() {
        onWindowResized.Destroy();
#if ENABLE_CHEATS
        ShutdownImGui();
#endif
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Renderer* renderer {};
    SDL_Window* window {};
    std::map<Renderer::TextureId, SDL_Texture*> textures;
    std::map<Renderer::TextId, std::string> fontPaths;
    std::map<std::pair<std::string, int>, TTF_Font*> fontCache;

    int renderOutputSizeW, renderOutputSizeH;

    Events::Handler onWindowResized;
};

void Renderer::Initialize(int32_t windowWidth, int32_t windowHeight) {
    RenderManager::instance().Initialize(windowWidth, windowHeight);
}

void Renderer::Destroy() {
    RenderManager::instance().Destroy();
}

void Renderer::SubscribeToEvents() {
    RenderManager::instance().SubscribeToEvents();
}

void Renderer::DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height) {
    RenderManager::instance().DrawSprite(textureId, x, y, width, height);
}

void Renderer::DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height, double angle) {
    RenderManager::instance().DrawSprite(textureId, x, y, width, height, angle);
}

void Renderer::DrawSpriteScreen(Renderer::TextureId textureId, float screenX, float screenY, float width, float height) {
    RenderManager::instance().DrawSpriteScreen(textureId, screenX, screenY, width, height);
}

void Renderer::DrawSpriteScreen(Renderer::TextureId textureId, float screenX, float screenY, float width, float height, double angle) {
    RenderManager::instance().DrawSpriteScreen(textureId, screenX, screenY, width, height, angle);
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

void Renderer::LoadAllFonts(const std::string& directory) {
    RenderManager::instance().LoadAllFonts(directory);
}

void Renderer::DrawText(Renderer::TextId fontId, const std::string& text, float x, float y, int fontSize, const SDL_Color* color, int maxLineWidth) {
    RenderManager::instance().DrawText(fontId, text, x, y, fontSize, color, maxLineWidth);
}

void Renderer::DrawTextScreen(Renderer::TextId fontId, const std::string& text, float screenX, float screenY, int fontSize, const SDL_Color* color, int maxLineWidth) {
    RenderManager::instance().DrawTextScreen(fontId, text, screenX, screenY, fontSize, color, maxLineWidth);
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

void Renderer::ProcessImGuiEvent(SDL_Event* event) {
    RenderManager::instance().ProcessImGuiEvent(event);
}

void Renderer::BeginImGuiFrame() {
    RenderManager::instance().BeginImGuiFrame();
}

void Renderer::EndImGuiFrame() {
    RenderManager::instance().EndImGuiFrame();
}