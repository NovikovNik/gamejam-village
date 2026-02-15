#include "./LevelLoader.h"
#include "./Game.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyBoardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectTileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/HealthBarComponent.h"
#include "../Components/ScriptComponent.h"
#include "glm/fwd.hpp"
#include <fstream>
#include <sstream>
#include <string>

int Game::mapHeight;
int Game::mapWidth;

LevelLoader::LevelLoader() {
    Logger::Log("LevelLoader constructor called!");
}

LevelLoader::~LevelLoader() {
    Logger::Log("LevelLoader destructor called!");
}

void LevelLoader::LoadTileMap(Registry& registry, AssetManager& assetManager, const std::string& filePath) {
    char* basePath = SDL_GetBasePath();
    std::string base = basePath ? basePath : "";
    SDL_free(basePath);

    std::ifstream mapFile(base + filePath);

    const int tileSize = 32;
    const double tileScale = 2.0;

    SDL_Texture* tilemapTexture = assetManager.GetTexture("tilemap-image");
    int tilemapWidth = 0, tilemapHeight = 0;
    SDL_QueryTexture(tilemapTexture, nullptr, nullptr, &tilemapWidth, &tilemapHeight);
    const int numTilesRow = tilemapWidth / tileSize;

    int rows = 0;
    int maxCols = 0;

    if (!mapFile.is_open()) {
        Logger::Err("Failed to open map file");
        return;
    }

    std::string line;
    while (std::getline(mapFile, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string token;

        int col = 0;
        while (std::getline(ss, token, ',')) {
            if (token.empty()) continue; // на случай "1,2,3," в конце

            int tileId = std::stoi(token);

            Entity tileEntity = registry.CreateEntity();
            tileEntity.Group("tile");
            tileEntity.AddComponent<TransformComponent>(
                glm::vec2(col * (tileSize * tileScale), rows * (tileSize * tileScale)),
                glm::vec2(tileScale, tileScale),
                0.0
            );
            tileEntity.AddComponent<SpriteComponent>(
                "tilemap-image",
                0, // zindex
                tileSize, tileSize,
                false,
                (tileId % numTilesRow) * tileSize,
                (tileId / numTilesRow) * tileSize
            );

            col++;
        }

        maxCols = std::max(maxCols, col);
        rows++;
    }

    mapFile.close();

    Game::mapWidth  = maxCols * tileSize * tileScale;
    Game::mapHeight = rows    * tileSize * tileScale;
}

void LevelLoader::LoadLevel(Registry& registry, AssetManager& assetManager, SDL_Renderer* renderer, sol::state& lua, std::string levelName) {
    sol::load_result script =  lua.load_file("./assets/scripts/" + levelName + ".lua");
    if (!script.valid()) {
        // Check syntax but does not execute it
        sol::error err = script;
        std::string errorMessage = err.what();
        Logger::Err("Error while loading level [" + levelName + "]");
        Logger::Err(errorMessage);
        return;
    }

    lua.script_file("./assets/scripts/" + levelName + ".lua");
    sol::table level = lua["Level"];
    Logger::Log("Level opened: [" + levelName + "]");

    // Read the level assets
    sol::table assets = level["assets"];
    int i = 0;
    while(true) {
        sol::optional<sol::table> hasAsset = assets[i];
        if (hasAsset == sol::nullopt) {
            break;
        }
        sol::table asset = hasAsset.value();
        std::string assetType = asset["type"];
        if (assetType == "texture") {
            assetManager.AddTexture(renderer, asset["id"], asset["file"]);
        } else if (assetType == "font") {
            assetManager.AddFont(asset["id"], asset["file"], asset["size"]);
        }
        i++;
    }

    LoadTileMap(registry, assetManager, level["tilemap"]);

    sol::table entities = level["entities"];
    i = 0;
    while(true) {
        sol::optional<sol::table> hasEntity = entities[i];
        if (hasEntity == sol::nullopt) {
            break;
        }
        sol::table entityLua = entities[i];
        Entity entity = registry.CreateEntity();

        sol::optional<std::string> tag = entityLua["tag"];
        if (tag != sol::nullopt) {
            entity.Tag(entityLua["tag"]);
        }
        sol::optional<std::string> group = entityLua["group"];
        if (group != sol::nullopt) {
            entity.Group(entityLua["group"]);
        }

        sol::optional<sol::table> hasComponents = entityLua["components"];
        if (hasComponents != sol::nullopt) {
            sol::table components = hasComponents.value();

            Logger::Log("Adding Transform");
            sol::optional<sol::table> hasTransform = components["transform_component"];
            if (hasTransform.has_value()) {
                sol::table transform = hasTransform.value();
                entity.AddComponent<TransformComponent>(
                    glm::vec2(
                        transform["x"],
                        transform["y"]
                    ),
                    glm::vec2(
                        transform["scale_x"],
                        transform["scale_y"]
                    ),
                    transform["angle"]
                );
            }
            Logger::Log("Adding RigidBody");
            sol::optional<sol::table> hasRigidBody = components["rigidbody_component"];
            if (hasRigidBody.has_value()) {
                sol::table rigidbody = hasRigidBody.value();
                entity.AddComponent<RigidBodyComponent>(
                    glm::vec2(
                        rigidbody["x"],
                        rigidbody["y"]
                    )
                );
            }
            Logger::Log("Adding Sprite");
            sol::optional<sol::table> hasSpriteComponent = components["sprite_component"];
            if (hasSpriteComponent.has_value()) {
                sol::table spriteComponent = hasSpriteComponent.value();
                entity.AddComponent<SpriteComponent>(
                    spriteComponent["id"],
                    spriteComponent["z_index"],
                    spriteComponent["h"],
                    spriteComponent["w"],
                    spriteComponent["is_fixed"]
                );
            }
            Logger::Log("Adding Projectile Emitter");
            sol::optional<sol::table> hasProjectTileEmitterComponent = components["emitter_component"];
            if (hasProjectTileEmitterComponent.has_value()) {
                sol::table tileEmitterComponent = hasProjectTileEmitterComponent.value();
                entity.AddComponent<ProjectTileEmitterComponent>(
                    tileEmitterComponent["speed"],
                    tileEmitterComponent["frequency"],
                    tileEmitterComponent["duration"],
                    tileEmitterComponent["damage"],
                    tileEmitterComponent.get_or("is_friendly", false),
                    tileEmitterComponent.get_or("auto_fire", true)
                );
            }
            Logger::Log("Adding Animation");
            sol::optional<sol::table> hasAnimationComponent = components["animation_component"];
            if (hasAnimationComponent.has_value()) {
                sol::table animationComponent = hasAnimationComponent.value();
                entity.AddComponent<AnimationComponent>(
                    animationComponent["frames"],
                    animationComponent["curr_frame"],
                    animationComponent["speed"],
                    animationComponent["is_loop"]
                );
            }
            Logger::Log("Adding Control");
            sol::optional<sol::table> hasKeyBoardControls = components["control_component"];
            if (hasKeyBoardControls.has_value()) {
                sol::table controlComponent = hasKeyBoardControls.value();
                entity.AddComponent<KeyBoardControlledComponent>(
                    glm::vec2(controlComponent["up_velocity"]["x"], controlComponent["up_velocity"]["y"]),
                    glm::vec2(controlComponent["right_velocity"]["x"], controlComponent["right_velocity"]["y"]),
                    glm::vec2(controlComponent["down_velocity"]["x"], controlComponent["down_velocity"]["y"]),
                    glm::vec2(controlComponent["left_velocity"]["x"], controlComponent["left_velocity"]["y"])
                );
            }
            Logger::Log("Adding Camera");
            sol::optional<bool> hasCameraComponent = components["camera_component"];
            if (hasCameraComponent.has_value()) {
                entity.AddComponent<CameraFollowComponent>();
            }
            Logger::Log("Adding BoxCollider");
            sol::optional<sol::table> hasBoxColliderComponent = components["boxcollider_component"];
            if (hasBoxColliderComponent.has_value()) {
                sol::table boxColliderComponent = hasBoxColliderComponent.value();
                entity.AddComponent<BoxColliderComponent>(
                    boxColliderComponent["size"]["x"],
                    boxColliderComponent["size"]["y"],
                    glm::vec2(boxColliderComponent["offset"]["x"], boxColliderComponent["offset"]["y"])
                );
            }
            Logger::Log("Adding Health");
            sol::optional<sol::table> hasHealthComponent = components["health_component"];
            if (hasHealthComponent.has_value()) {
                sol::table healthComponent = hasHealthComponent.value();
                entity.AddComponent<HealthComponent>(
                    static_cast<int>(healthComponent["value"])
                );
            }
            Logger::Log("Adding HealthBar");
            sol::optional<bool> hasHealthBarComponent = components["healthbar_component"];
            if (hasHealthBarComponent.has_value()) {
                entity.AddComponent<HealthBarComponent>();
            }
            Logger::Log("Adding FontComponent");
            sol::optional<sol::table> hasLabelComponent = components["label_component"];
            if (hasLabelComponent.has_value()) {
                sol::table labelComponent = hasLabelComponent.value();
                SDL_Color white_c = {255, 255, 255};
                entity.AddComponent<TextLabelComponent>(
                    glm::vec2(labelComponent["position"]["x"], labelComponent["position"]["y"]),
                    labelComponent["text"],
                    labelComponent["font"],
                    white_c
                );
            }
            Logger::Log("Adding ScriptComponent");
            sol::optional<sol::table> script = components["on_update_script"];
            if (script.has_value()) {
                sol::function func = script.value()[0];
                entity.AddComponent<ScriptComponent>(func);
            }
        }
        i++;

    }

    // Entity chopper = registry.CreateEntity();
    // chopper.AddComponent<TransformComponent>(glm::vec2(100, 100), glm::vec2(1, 1), 0.0);
    // chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    // chopper.AddComponent<SpriteComponent>("chopper-image", 2, 32, 32, false);
    // chopper.AddComponent<ProjectTileEmitterComponent>(150, 500, 10000, 10, true, false);
//     chopper.AddComponent<AnimationComponent>(2, 0, 15, true);
//     chopper.AddComponent<KeyBoardControlledComponent>(glm::vec2(0, -80), glm::vec2(80, 0), glm::vec2(0, 80), glm::vec2(-80, 0));
//     chopper.AddComponent<CameraFollowComponent>();
//     chopper.AddComponent<BoxColliderComponent>(32, 32);
//     chopper.AddComponent<HealthComponent>(100);
//     chopper.AddComponent<HealthBarComponent>();
//     chopper.Tag("Player");

//     Entity radar = registry.CreateEntity();
//     radar.AddComponent<TransformComponent>(glm::vec2(Game::windowWidth -74, 10.0), glm::vec2(1, 1), 0.0);
//     radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
//     radar.AddComponent<SpriteComponent>("radar", 2, 64, 64, true);
//     radar.AddComponent<AnimationComponent>(8, 0, 5, true);

//     Entity treeB = registry.CreateEntity();
//     treeB.AddComponent<TransformComponent>(glm::vec2(470, 500), glm::vec2(1, 1), 0.0);
//     treeB.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
//     treeB.AddComponent<SpriteComponent>("tree-image", 1, 16, 32, false);
//     treeB.AddComponent<BoxColliderComponent>(16, 32);
//     treeB.Group("obstacles");

//     Entity tank = registry.CreateEntity();
//     tank.AddComponent<TransformComponent>(glm::vec2(380, 500), glm::vec2(1, 1), 0.0);
//     tank.AddComponent<RigidBodyComponent>(glm::vec2(20.0, 0.0));
//     tank.AddComponent<SpriteComponent>("tank-image", 1, 32, 32, false);
//     tank.AddComponent<BoxColliderComponent>(32, 32);
//     // tank.AddComponent<ProjectTileEmitterComponent>(100, 5000, 10000, 10, false);
//     tank.AddComponent<HealthComponent>(100);
//     tank.AddComponent<HealthBarComponent>();
//     tank.Group("enemies");

//     Entity truck = registry.CreateEntity();
//     truck.AddComponent<TransformComponent>(glm::vec2(420, 400), glm::vec2(1, 1), 0.0);
//     truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
//     truck.AddComponent<SpriteComponent>("truck-image", 1, 32, 32, false);
//     truck.AddComponent<BoxColliderComponent>(32, 32);
//     // truck.AddComponent<ProjectTileEmitterComponent>(100, 2000, 10000, 10, false);
//     truck.AddComponent<HealthComponent>(100);
//     truck.AddComponent<HealthBarComponent>();
//     truck.Group("enemies");

//     Entity treeA = registry.CreateEntity();
//     treeA.AddComponent<TransformComponent>(glm::vec2(250, 505), glm::vec2(1, 1), 0.0);
//     treeA.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
//     treeA.AddComponent<SpriteComponent>("tree-image", 1, 16, 32, false);
//     treeA.AddComponent<BoxColliderComponent>(16, 32);
//     treeA.Group("obstacles");

//     Entity label = registry.CreateEntity();
//     SDL_Color white_c = {255, 255, 255};
//     label.AddComponent<TextLabelComponent>(glm::vec2(Game::windowWidth / 2 - 40, 20), "ep_Chopper 1,0", "pico8", white_c);
}
