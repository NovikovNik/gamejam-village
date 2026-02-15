#pragma once

#include <string>
#include "../ECS/ECS.h"
#include "../AssetManager/AssetManager.h"
#include "SDL_render.h"
#include <SDL.h>
#include <sol/sol.hpp>

class LevelLoader {
    public:
    LevelLoader();
    ~LevelLoader();

    void LoadLevel(Registry& registry, AssetManager& assetManager, SDL_Renderer* renderer, sol::state& lua, std::string levelName);
    void LoadTileMap(Registry& registry, AssetManager& assetManager, const std::string& filePath);
};
