#include "Renderer.h"
#include <Utils/Singleton.h>
#include "../Logger/Logger.h"

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

        if (!SDL_CreateWindowAndRenderer("test", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
            SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
            return;
        }
        SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_DISABLED);
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
        rect.x = x;
        rect.y = y;
        rect.w = width;
        rect.h = height;
        SDL_RenderTexture(renderer, texture, NULL, &rect);
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

    void Destroy() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Renderer* renderer {};
    SDL_Window* window {};
    std::map<Renderer::TextureId, SDL_Texture*> textures;
};

void Renderer::Initialize(int32_t windowWidth, int32_t windowHeight) {
    RenderManager::instance().Initialize(windowWidth, windowHeight);
}

void Renderer::Destroy() {
    RenderManager::instance().Destroy();
}

void Renderer::DrawSprite(Renderer::TextureId textureId, float x, float y, float width, float height) {
    RenderManager::instance().DrawSprite(textureId, x, y, width, height);
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
