#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include "../EventBus/EventBus.h"
#include <imgui/imgui.h>
#include "../Events/GameShutdownEvent.h"

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
    private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect camera;

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

    private:
        void OnGameShutdown(GameShutdownEvent& e);

    public:
        static int windowWidth;
        static int windowHeight;
        static int windowLogicWidth;
        static int windowLogicHeight;
        static int mapWidth;
        static int mapHeight;

    private:
        Events::Handler onGameShutdown;
};
