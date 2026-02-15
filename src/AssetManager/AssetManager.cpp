#include "./AssetManager.h"
#include "../Logger/Logger.h"
#include "SDL_image.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_ttf.h"

AssetManager::AssetManager() {
    SDL_free(basePath);
    Logger::Log("AssetManager constructor called");
}

AssetManager::~AssetManager() {
    ClearAssets();
    Logger::Log("AssetManager destructor called");
}

void AssetManager::ClearAssets() {
    for (auto& texture: textures) {
        SDL_DestroyTexture(texture.second);
    }
    textures.clear();

    for (auto& font: fonts) {
        TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void AssetManager::AddTexture(SDL_Renderer* renderer,const std::string& assetId, const std::string& filePath) {
    SDL_Surface* surface = IMG_Load((base + filePath).c_str());
    if (!surface) {
        Logger::Err("Failed to load surface '" + filePath + "'");
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        Logger::Err("Failed to load texture '" + filePath + "'");
    }
    SDL_FreeSurface(surface);

    textures.emplace(assetId, texture);
    Logger::Log("Texture added to asset manager: " + assetId);
}

SDL_Texture* AssetManager::GetTexture(const std::string& assetId) {
    return textures[assetId];
}

void AssetManager::AddFont(const std::string& assetId, const std::string& filePath, int fontSize) {
    fonts.emplace(assetId, TTF_OpenFont(filePath.c_str(), fontSize));
}

TTF_Font* AssetManager::GetFont(const std::string assetId) {
    return fonts[assetId];
}
