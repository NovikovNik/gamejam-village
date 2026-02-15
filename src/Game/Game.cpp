#include "Game.h"
#include "GameFeatures.h"
#include "LevelLoader.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "SDL_render.h"
#include <SDL_video.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardMovementSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectTileEmitterSystem.h"
#include "../Systems/ProjecTileLifeCycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/HealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
#include "../Systems/ScriptSystem.h"
#include "SDL_timer.h"
#include "SDL_ttf.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

// Remove SDL.h from here; forward declare in Game.h if needed.

int Game::windowHeight;
int Game::windowWidth;

Game::Game() {
    registry = std::make_unique<Registry>();
    assetManager = std::make_unique<AssetManager>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called");
}

Game::~Game() {
    Logger::Log("Game destructor called");
}

void Game::Initialize() {
    isRunning = false;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Err("SDL_Init failed: " + std::string(SDL_GetError()));
        return;
    }

    if (TTF_Init() != 0) {
        Logger::Err("TTF_Init failed: " + std::string(SDL_GetError()));
        return;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    windowWidth = 800; //displayMode.w;
    windowHeight = 600; //displayMode.h;

    window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        Logger::Err("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        return;
    }
    uint32_t renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, renderFlags);
    if (!renderer) {
        Logger::Err("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
        return;
    }
    SDL_RenderSetLogicalSize(renderer, 800, 600);
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Init the Camera with entire screen area
    camera.x = 0;
    camera.y = 0;
    camera.w = windowWidth;
    camera.h = windowHeight;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Init ImGui rendere
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    isRunning = true;
}

void Game::Setup() {
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderColliderSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyBoardMovementSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectTileEmitterSystem>();
    registry->AddSystem<ProjectTileLifeCycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<HealthBarSystem>();
    registry->AddSystem<RenderGUISystem>();
    registry->AddSystem<ScriptSystem>();

    // Create C++ / Lua bindings
    registry->GetSystem<ScriptSystem>().CreateLuaBindings(lua);

    LevelLoader loader;
    lua.open_libraries(sol::lib::base, sol::lib::math);
    loader.LoadLevel(*registry, *assetManager, renderer, lua, "Level1");
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
    lua.clear_package_loaders();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::ProcessInput() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        int mouseX, mouseY;
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2(mouseX, mouseY);
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        switch(event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    if (GameFeatures::isDebug) {
                        GameFeatures::isDebug = false;
                        break;
                    }
                    isRunning = false;
                }
                if(event.key.keysym.sym == SDLK_d) {
                    GameFeatures::isDebug = !GameFeatures::isDebug;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    eventBus->EmitEvent<PlayerAttackEvent>();
                }
                break;
            default:
                eventBus->EmitEvent<KeyPressedEvent>(event.key.keysym.sym);
        }
    }
}

void Game::Update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks64() - millisecPreviousFrame);
    if ((timeToWait > 0) && (timeToWait <= MILLISECS_PER_FRAME)) {
        SDL_Delay(timeToWait);
    }
    // The difference in ticks since the last frame converted to seconds;
    deltaTime = (SDL_GetTicks() - millisecPreviousFrame) / 1000.0;
    millisecPreviousFrame = SDL_GetTicks();

    // Reset all event handlers
    eventBus->Reset();

    // Perform the subscription
    registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<KeyBoardMovementSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<ProjectTileEmitterSystem>().SubscribeToEvents(eventBus);

    // Update registry to process entities that are waiting to be created/deleted
    registry->Update();

    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(eventBus);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectTileEmitterSystem>().Update(*registry);
    registry->GetSystem<ProjectTileLifeCycleSystem>().Update();
    registry->GetSystem<ScriptSystem>().Update(deltaTime, SDL_GetTicks64());
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    // TODO: Render game objects (entities)
    registry->GetSystem<RenderSystem>().Update(renderer, *assetManager, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, *assetManager, camera);
    registry->GetSystem<HealthBarSystem>().Update(renderer, *assetManager, camera);

    if (GameFeatures::isDebug) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        registry->GetSystem<RenderGUISystem>().Update(renderer, *registry, camera);
    }
    SDL_RenderPresent(renderer);
}
