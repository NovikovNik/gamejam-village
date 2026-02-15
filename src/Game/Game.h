#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include "../EventBus/EventBus.h"
#include <imgui/imgui.h>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
    private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect camera;
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
