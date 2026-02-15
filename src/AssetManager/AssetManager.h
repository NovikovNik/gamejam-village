#pragma once

#include <unordered_map>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class AssetManager {
    private:
        std::unordered_map<std::string, SDL_Texture*> textures;
        std::unordered_map<std::string, TTF_Font*> fonts;

        char* basePath = SDL_GetBasePath();
        std::string base = basePath ? basePath : "";

    public:
        AssetManager();
        ~AssetManager();

        void ClearAssets();
        void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
        SDL_Texture* GetTexture(const std::string& assetId);

        void AddFont(const std::string& assetId, const std::string& filePath, int fontSize);
        TTF_Font* GetFont(const std::string assetId);
};
