#pragma once

#include <SDL.h>
#include <sol/sol.hpp>
#include <memory>
#include "../ECS/ECS.h"
#include "../AssetManager/AssetManager.h"
#include "../EventBus/EventBus.h"
#include <imgui/imgui.h>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
    private:
    sol::state lua;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect camera;
    std::unique_ptr<Registry> registry;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<EventBus> eventBus;

    bool isRunning;
    int millisecPreviousFrame = 0.0;
    double deltaTime;
        //..
    public:
        Game();
        ~Game();
        void Initialize();
        void Setup();
        void Run();
        void ProcessInput();
        void Update();
        void Render();
        void Destroy();

    public:
        static int windowWidth;
        static int windowHeight;
        static int mapWidth;
        static int mapHeight;
};
