#include "Game.h"
#include "GameFeatures.h"
#include "../Logger/Logger.h"
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <cstddef>
#include <glm/glm.hpp>
#include <SDL3/SDL_timer.h>
#include <SDL3_ttf/SDL_ttf.h>
// #include <imgui/imgui.h> need to adapt
// #include <imgui/imgui_impl_sdl2.h>
// #include <imgui/imgui_impl_sdlrenderer2.h>

// Remove SDL.h from here; forward declare in Game.h if needed.

int Game::windowHeight;
int Game::windowWidth;
int Game::windowLogicHeight;
int Game::windowLogicWidth;

Game::Game() {
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called");
}

Game::~Game() {
    Logger::Log("Game destructor called");
}

void Game::Initialize() {
    isRunning = false;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Logger::Err("SDL_Init failed: " + std::string(SDL_GetError()));
        return;
    }

    if (!TTF_Init()) {
        Logger::Err("TTF_Init failed: " + std::string(SDL_GetError()));
        return;
    }

    windowWidth = 800;          //displayMode.w;
    windowHeight = 600;         //displayMode.h;
    windowLogicWidth = 800;     // Render will be in this resolution despite the window size
    windowLogicHeight = 600;    // Render will be in this resolution despite the window size

    if (!SDL_CreateWindowAndRenderer("test", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return;
    }
    SDL_SetRenderLogicalPresentation(renderer, windowLogicWidth, windowLogicHeight, SDL_LOGICAL_PRESENTATION_DISABLED);
    // Fullscreen settings
    if (GameFeatures::isFullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    } else {
        Logger::Debug("Fullscreen was disabled in GameFeatures.h");
    }

    // Init the Camera with entire screen area
    camera.x = 0;
    camera.y = 0;
    camera.w = windowWidth;
    camera.h = windowHeight;

    // IMGUI_CHECKVERSION(); Need to update to SDL3
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // // Setup Dear ImGui style
    // ImGui::StyleColorsDark();

    // // Init ImGui rendere
    // ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    // ImGui_ImplSDLRenderer2_Init(renderer);

    isRunning = true;
}

void Game::Setup() {
    // Here initial setup for future game
}

void Game::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::Destroy() {
    // The order is strict because of asserts
    // ImGui_ImplSDLRenderer2_Shutdown();
    // ImGui_ImplSDL2_Shutdown();
    // ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::ProcessInput() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        // ImGui_ImplSDL2_ProcessEvent(&event);

        // int mouseX, mouseY;
        // const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        // ImGuiIO& io = ImGui::GetIO();
        // io.MousePos = ImVec2(mouseX, mouseY);
        // io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        // io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        switch(event.type) {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if(event.key.key == SDLK_ESCAPE) {
                    if (GameFeatures::isDebug) {
                        GameFeatures::isDebug = false;
                        break;
                    }
                    isRunning = false;
                }
                if(event.key.key == SDLK_TAB) {
                    GameFeatures::isDebug = !GameFeatures::isDebug;
                    Logger::Debug("Debug state changed to: " + std::to_string(GameFeatures::isDebug));
                }
                break;
            default:
                break;
        }
    }
}

void Game::Update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecPreviousFrame);
    if ((timeToWait > 0) && (timeToWait <= MILLISECS_PER_FRAME)) {
        SDL_Delay(timeToWait);
    }
    // The difference in ticks since the last frame converted to seconds;
    deltaTime = (SDL_GetTicks() - millisecPreviousFrame) / 1000.0;
    millisecPreviousFrame = SDL_GetTicks();

    // Reset all event handlers
    eventBus->Reset();
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    if (GameFeatures::isDebug) {
        // Usually we should render here some debug infos
    }

    // THIS BLOCK SHOULD BE DELETED. ONLY AS EXAMPLE!!
    SDL_Texture* texture = IMG_LoadTexture(renderer, "assets/test.png");
    if (!texture) {
        SDL_Log("Texture creation failed: %s", SDL_GetError());
    }

    // Получаем размеры текстуры в SDL3
    float texW, texH;
    SDL_GetTextureSize(texture, &texW, &texH);

    SDL_FRect destRect;
    destRect.w = texW;
    destRect.h = texH;
    destRect.x = (windowWidth - destRect.w) / 2.0f;
    destRect.y = (windowHeight - destRect.h) / 2.0f;

    SDL_RenderTexture(renderer, texture, NULL, &destRect);
    // THIS BLOCK SHOULD BE DELETED. ONLY AS EXAMPLE!!

    SDL_RenderPresent(renderer);
}
