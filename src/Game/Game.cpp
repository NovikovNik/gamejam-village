#include "Game.h"

#include <Map/Map.h>
#include <Renderer/Renderer.h>
#include <FileSystem/FileSystem.h>
#include <ProgressSystem/ProgressSystem.h>
#include <DialogSystem/DialogSystem.h>
#include <Game/GameplayLogic.h>

#include "Game/GameStates.h"
#include "GameFeatures.h"
#include <EventBus/EventBus.h>
#include "../Events/WindowResizedEvent.h"
#include "../Events/WindowFocusedEvent.h"
#include "../Events/WindowUnfocusedEvent.h"
#include "../Events/InterectButtonPressedEvent.h"
#include "../Events/NextDialogLineEvent.h"
#include "../Events/GameShutdownEvent.h"
#include "../Gameplay/WorldState.h"
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
    Logger::Log("[Game] Constructor called");
}

Game::~Game() {
    Logger::Log("[Game] Game destructor called");
}

void Game::Initialize() {

    Renderer::Initialize(windowWidth, windowHeight);
    ProgressSystemManager::Initialize();
    GameplayLogic::Initialize();
    DialogSystemManager::Initialize();
    DialogSystemManager::LoadAllDialogs("assets/dialogs/");
    Renderer::LoadAllTextures("assets/textures/");
    Renderer::LoadAllFonts("assets/fonts/");
    // Стартовую локацию выставляем в сохранке!!!
    bool isLoaded = MapManager::LoadLastLoadedLevel();
    if (!isLoaded) {
        Logger::Err("Failed to load map");
        return;
    } else {
        isRunning = true;
    }
    WorldState::Initiate();

    // В редких ситуациях из игрового процесса придет сигнал на выход из игры
    // Например, если игрок создаст файл kick-my.ass в world-entry-2 локации
    onGameShutdown = EventBus::instance().SubscribeToEvent<GameShutdownEvent>(this, &Game::OnGameShutdown);
}

void Game::OnGameShutdown(GameShutdownEvent& e) {
    Logger::Log("[Game] Game shutdown event received. Stopping the game.");
    isRunning = false;
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
    WorldState::Destroy();
    DialogSystemManager::Destroy();
    GameplayLogic::Destroy();
}

void Game::ProcessInput() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        // Let ImGui process events first
        Renderer::ProcessImGuiEvent(&event);

        switch(event.type) {
            case SDL_EVENT_QUIT:
                // Перед выходом сохраняем прогресс в файл!
                ProgressSystemManager::SaveData();
                isRunning = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                Logger::Debug("[Game/SDL] Window resized event received");
                EventBus::instance().EmitEvent<WindowResizedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                Logger::Debug("[Game/SDL] Window focus event received");
                EventBus::instance().EmitEvent<WindowFocusedEvent>();
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                Logger::Debug("[Game/SDL] Window unfocus event received");
                EventBus::instance().EmitEvent<WindowUnfocusedEvent>();
                break;
            case SDL_EVENT_KEY_DOWN:
                // Handle tilde key to toggle cheats (always processed)
#if ENABLE_CHEATS
                if(event.key.key == SDLK_GRAVE) {
                    Cheats::ToggleCheats();
                    Logger::Debug("Cheats toggled: " + std::to_string(Cheats::AreCheatsActive()));
                    break;
                }
                
                // If cheats are active, ignore all other keyboard input
                if (Cheats::AreCheatsActive()) {
                    break;
                }
#endif
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
                if (event.key.key == SDLK_O) { // Open current location
                    MapManager::OpenCurrentLocationInExplorer();
                    break;
                }
                if (event.key.key == SDLK_R) {
                    // RELOAD LEVEL!
                    MapManager::ReloadMap();
                    break;
                }
                if (event.key.key == SDLK_F) {
                    EventBus::instance().EmitEvent<InterectButtonPressedEvent>();
                    break;
                }
                if (event.key.key == SDLK_SPACE) {
                    EventBus::instance().EmitEvent<NextDialogLineEvent>();
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
#if ENABLE_CHEATS
                if (Cheats::AreCheatsActive()) {
                    break;
                }
#endif
                
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
    GameplayLogic::Update();

    // Dispatch all events in the queue
//    EventsQueue::instance().Dispatch();
}

void Game::Render() {

    Renderer::BeginRender();
    
    // Render game content
    MapManager::Render(deltaTime);
    // Тут точно дельта не нужна
    // Но мб будем что-то двигать? И тогда будет нужна,
    // Но пока пофиг
    DialogSystemManager::RenderDialog();

#if ENABLE_CHEATS
    Cheats::UpdateAndRender();
#endif
    Renderer::EndRender();
}
