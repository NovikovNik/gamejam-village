#include "Game.h"

#include <Map/Map.h>
#include <Renderer/Renderer.h>
#include <FileSystem/FileSystem.h>

#include "Game/GameStates.h"
#include "GameFeatures.h"
#include "../Events/WindowResizedEvent.h"
#include "../Events/WindowFocusedEvent.h"
#include "../Events/WindowUnfocusedEvent.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_oldnames.h"
//#include "../Logger/Logger.h"
//#include <SDL3/SDL_keycode.h>
//#include <SDL3/SDL_mouse.h>
//#include <SDL3/SDL_render.h>
//#include <SDL3/SDL_video.h>
//#include <SDL3_image/SDL_image.h>
//#include <glm/glm.hpp>
//#include <SDL3/SDL_timer.h>
//#include <SDL3_ttf/SDL_ttf.h>
#include <cstddef>
// #include <imgui/imgui.h> need to adapt
// #include <imgui/imgui_impl_sdl2.h>
// #include <imgui/imgui_impl_sdlrenderer2.h>

// Remove SDL.h from here; forward declare in Game.h if needed.

int Game::windowHeight = 600;
int Game::windowWidth = 800;
// int Game::windowLogicHeight;
// int Game::windowLogicWidth;

Game::Game() {
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called");
}

Game::~Game() {
    Logger::Log("Game destructor called");
}

void Game::Initialize() {

    Renderer::Initialize(windowWidth, windowHeight);
    Renderer::LoadAllTextures("assets/textures/");
    bool isLoaded = MapManager::LoadMap("assets/maps/world-1.json");
    if (!isLoaded) {
        Logger::Err("Failed to load map");
        return;
    } else {
        isRunning = true;
    }
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

    Renderer::Destroy();
}

void Game::ProcessInput() {
    // Некрасиво, но нужно подписать Renderer на события до старта событий собственно
    Renderer::SubscribeToEvents(eventBus);

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
            case SDL_EVENT_WINDOW_RESIZED:
                Logger::Debug("Window resized");
                eventBus->EmitEvent<WindowResizedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                Logger::Debug("Window focus gained");
                eventBus->EmitEvent<WindowFocusedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                Logger::Debug("Window focus lost");
                eventBus->EmitEvent<WindowUnfocusedEvent>();
                break;
            case SDL_EVENT_KEY_DOWN:
                if(event.key.key == SDLK_ESCAPE) {
                    if (GameFeatures::isDebug) {
                        GameFeatures::isDebug = false;
                        break;
                    }
                    isRunning = false;
                    break;
                }
                if(event.key.key == SDLK_TAB) {
                    GameFeatures::isDebug = !GameFeatures::isDebug;
                    Logger::Debug("Debug state changed to: " + std::to_string(GameFeatures::isDebug));

                    // FOR TEST!
                    FileSystemManager::OpenSystemExplorer("test");
                    break;
                }
                if (event.key.key == SDLK_W ||event.key.key == SDLK_UP) {
                    GameStates::instance().w = true;
                    break;
                }
                if (event.key.key == SDLK_S ||event.key.key == SDLK_DOWN) {
                    GameStates::instance().s = true;
                    break;
                }
                if (event.key.key == SDLK_D ||event.key.key == SDLK_RIGHT) {
                    GameStates::instance().d = true;
                    break;
                }
                if (event.key.key == SDLK_A ||event.key.key == SDLK_LEFT) {
                    GameStates::instance().a = true;
                    break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_W ||event.key.key == SDLK_UP) {
                    GameStates::instance().w = false;
                    break;
                }
                if (event.key.key == SDLK_S ||event.key.key == SDLK_DOWN) {
                    GameStates::instance().s = false;
                    break;
                }
                if (event.key.key == SDLK_D ||event.key.key == SDLK_RIGHT) {
                    GameStates::instance().d = false;
                    break;
                }
                if (event.key.key == SDLK_A ||event.key.key == SDLK_LEFT) {
                    GameStates::instance().a = false;
                    break;
                }
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

    MapManager::Update(deltaTime);

    // Reset all event handlers
    eventBus->Reset();
}

void Game::Render() {

    Renderer::BeginRender();
    MapManager::Render(deltaTime);
    Renderer::EndRender();


//    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
//    SDL_RenderClear(renderer);
//
//    if (GameFeatures::isDebug) {
//        // Usually we should render here some debug infos
//    }
//
//    // THIS BLOCK SHOULD BE DELETED. ONLY AS EXAMPLE!!
//    SDL_Texture* texture = IMG_LoadTexture(renderer, "assets/test.png");
//    if (!texture) {
//        SDL_Log("Texture creation failed: %s", SDL_GetError());
//    }
//
//    // Получаем размеры текстуры в SDL3
//    float texW, texH;
//    SDL_GetTextureSize(texture, &texW, &texH);
//
//    SDL_FRect destRect;
//    destRect.w = texW;
//    destRect.h = texH;
//    destRect.x = (windowWidth - destRect.w) / 2.0f;
//    destRect.y = (windowHeight - destRect.h) / 2.0f;
//
//    SDL_RenderTexture(renderer, texture, NULL, &destRect);
//    // THIS BLOCK SHOULD BE DELETED. ONLY AS EXAMPLE!!
//
//    SDL_RenderPresent(renderer);
}
