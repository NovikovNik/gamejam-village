#include "Game.h"

#include <Map/Map.h>
#include <Renderer/Renderer.h>
#include <FileSystem/FileSystem.h>

#include "Game/GameStates.h"
#include "GameFeatures.h"
#include <EventBus/EventsQueue.h>
#include "../Events/WindowResizedEvent.h"
#include "../Events/WindowFocusedEvent.h"
#include "../Events/WindowUnfocusedEvent.h"
#include "../Events/InterectButtonPressedEvent.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_oldnames.h"
#include <Tools/Cheats.h>
#include <cstddef>


// Remove SDL.h from here; forward declare in Game.h if needed.

int Game::windowHeight = 600;
int Game::windowWidth = 800;
// int Game::windowLogicHeight;
// int Game::windowLogicWidth;

Game::Game() {
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
    Renderer::Destroy();
}

void Game::ProcessInput() {
    // Некрасиво, но нужно подписать Renderer на события до старта событий собственно
    Renderer::SubscribeToEvents();

    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        // Let ImGui process events first
        Renderer::ProcessImGuiEvent(&event);

        switch(event.type) {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                Logger::Debug("[Game/SDL] Window resized");
                EventBus::instance().EmitEvent<WindowResizedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                Logger::Debug("[Game/SDL] Window focus gained");
                EventBus::instance().EmitEvent<WindowFocusedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                Logger::Debug("[Game/SDL] Window focus lost");
                EventBus::instance().EmitEvent<WindowUnfocusedEvent>();
                break;
            case SDL_EVENT_KEY_DOWN:
                // Handle tilde key to toggle cheats (always processed)
                if(event.key.key == SDLK_GRAVE) {
                    Cheats::ToggleCheats();
                    Logger::Debug("Cheats toggled: " + std::to_string(Cheats::AreCheatsActive()));
                    break;
                }
                
                // If cheats are active, ignore all other keyboard input
                if (Cheats::AreCheatsActive()) {
                    break;
                }
                
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
                    break;
                }
                if (event.key.key == SDLK_O) {
                    // FOR TEST!
                    FileSystemManager::CreateKeyFile("test", "box_1.spg");
                    FileSystemManager::OpenSystemExplorer("test");
                    break;
                }
                if (event.key.key == SDLK_R) {
                    // RELOAD LEVEL!
                    MapManager::ReloadMap();
                    break;
                }
                if (event.key.key == SDLK_F) {
                    EventsQueue::instance().Push(InterectButtonPressedEvent{});
                    break;
                }
                if (event.key.key == SDLK_W || event.key.key == SDLK_UP) {
                    if (GameFeatures::movementDirection == MovementDirection::FOUR_DIRECTIONS) {
                        GameStates::instance().SetOnlyW();
                    } else {
                        GameStates::instance().w = true;
                    }
                    break;
                }
                if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN) {
                    if (GameFeatures::movementDirection == MovementDirection::FOUR_DIRECTIONS) {
                        GameStates::instance().SetOnlyS();
                    } else {
                        GameStates::instance().s = true;
                    }
                    break;
                }
                if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {
                    if (GameFeatures::movementDirection == MovementDirection::FOUR_DIRECTIONS) {
                        GameStates::instance().SetOnlyD();
                    } else {
                        GameStates::instance().d = true;
                    }
                    break;
                }
                if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT) {
                    if (GameFeatures::movementDirection == MovementDirection::FOUR_DIRECTIONS) {
                        GameStates::instance().SetOnlyA();
                    } else {
                        GameStates::instance().a = true;
                    }
                    break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                // If cheats are active, ignore all keyboard releases
                if (Cheats::AreCheatsActive()) {
                    break;
                }
                
                if (event.key.key == SDLK_W || event.key.key == SDLK_UP ||
                    event.key.key == SDLK_S || event.key.key == SDLK_DOWN ||
                    event.key.key == SDLK_A || event.key.key == SDLK_LEFT ||
                    event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {
                    bool fourDir = (GameFeatures::movementDirection == MovementDirection::FOUR_DIRECTIONS);
                    GameStates::instance().SyncMovementFromKeyboard(fourDir);
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

    if (GameFeatures::isDebug) {
        static Uint32 lastTime = SDL_GetTicks();
        static int frames = 0;
        static float fps_live = 0.0f;
    
        Uint32 now = SDL_GetTicks();
        frames++;
    
        Uint32 elapsed = now - lastTime;
        if (elapsed >= 1000) {
            fps_live = frames * 1000.0f / (now - lastTime);
            frames = 0;
            lastTime = now;
            Renderer::PrintFPSinTitle(fps_live);
        }
    }

    MapManager::Update(deltaTime);

    // Dispatch all events in the queue
    EventsQueue::instance().Dispatch();

    // Reset all event handlers
    EventBus::instance().Reset();
}

void Game::Render() {

    Renderer::BeginRender();
    
    // Render game content
    MapManager::Render(deltaTime);
    

    Cheats::UpdateAndRender();
    Renderer::EndRender();
}
