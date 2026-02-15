#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/ProjectTileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ProjectTileComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/PlayerAttackEvent.h"
#include "glm/fwd.hpp"
#include <cmath>

class ProjectTileEmitterSystem : public System {
    public:
    ProjectTileEmitterSystem() {
        RequireComponent<ProjectTileEmitterComponent>();
        RequireComponent<TransformComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
        eventBus->SubscribeToEvent<PlayerAttackEvent, ProjectTileEmitterSystem>(this, &ProjectTileEmitterSystem::onPlayerAttackPressed);
    }

    void onPlayerAttackPressed(PlayerAttackEvent& event) {
        Logger::Log("Player attack emitted");
        for (auto entity: GetSystemEntities()) {
            if (!entity.HasTag("Player")) {
                return;
            }
            if (const auto& spriteComponent = entity.GetComponent<SpriteComponent>(); true) {
                const auto& transform = entity.GetComponent<TransformComponent>();
                auto& projectileEmitter = entity.GetComponent<ProjectTileEmitterComponent>();
                if (SDL_GetTicks64() - projectileEmitter.lastEmissionTime > projectileEmitter.repeatFreaquency){
                    glm::vec2 projectTilePosition = transform.position;
                    projectTilePosition.x += (transform.scale.x * spriteComponent.width /2);
                    projectTilePosition.y += (transform.scale.y * spriteComponent.height /2);

                    switch (spriteComponent.direction) {
                        case Direction::UP:
                            SpawnEntity(*entity.registry, projectTilePosition, projectileEmitter, 3.0f * M_PI / 2.0f);
                            break;
                        case Direction::RIGHT:
                            SpawnEntity(*entity.registry, projectTilePosition, projectileEmitter, 0.0f);
                            break;
                        case Direction::DOWN:
                            SpawnEntity(*entity.registry, projectTilePosition, projectileEmitter, M_PI / 2.0f);
                            break;
                        case Direction::LEFT:
                            SpawnEntity(*entity.registry, projectTilePosition, projectileEmitter, M_PI);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    void Update(Registry& registry) {
        for (auto entity: GetSystemEntities()) {
            auto& projectileEmitter = entity.GetComponent<ProjectTileEmitterComponent>();
            const auto& transform = entity.GetComponent<TransformComponent>();

            if (projectileEmitter.autoFire){
                if (SDL_GetTicks64() - projectileEmitter.lastEmissionTime > projectileEmitter.repeatFreaquency) {
                    glm::vec2 projectTilePosition = transform.position;
                    float projectTileAngle = 0.0;
                    glm::vec2 delta = glm::vec2(0);
                    std::optional<Entity> playerEntity = registry.GetEntityByTag("Player");

                    if (entity.HasComponent<SpriteComponent>()) {
                        auto sprite = entity.GetComponent<SpriteComponent>();
                        projectTilePosition.x += (transform.scale.x * sprite.width /2);
                        projectTilePosition.y += (transform.scale.y * sprite.height /2);
                    }

                    if (playerEntity.has_value()){
                        const auto& playerTransformComponent = playerEntity.value().GetComponent<TransformComponent>();
                        const auto& playerSpriteComponent = playerEntity.value().GetComponent<SpriteComponent>();
                        delta = glm::vec2(
                            playerTransformComponent.position.x + playerSpriteComponent.width * 0.5 - projectTilePosition.x,
                            playerTransformComponent.position.y + playerSpriteComponent.height * 0.5 - projectTilePosition.y
                        );

                        projectTileAngle = std::atan2(delta.y, delta.x);
                    }
                    // add projectile
                    SpawnEntity(registry, projectTilePosition, projectileEmitter, projectTileAngle);
            }
            }
        }
    }

    void SpawnEntity(Registry& registry, glm::vec2 projectTilePosition, ProjectTileEmitterComponent& projectileEmitter, float angle = 0) {
        Entity projectile = registry.CreateEntity();
        projectile.Group("projectiles");

        projectile.AddComponent<TransformComponent>(projectTilePosition, glm::vec2(1.0, 1.0), 0);
        projectile.AddComponent<RigidBodyComponent>(glm::vec2(projectileEmitter.speed * cos(angle) - 0 * sin(angle),
            projectileEmitter.speed * sin(angle)));
        projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
        projectile.AddComponent<BoxColliderComponent>(4, 4);
        projectile.AddComponent<ProjectTileComponent>(projectileEmitter.isFriendly,
            projectileEmitter.hitPercentDamage,
            projectileEmitter.projectileDuration);

        projectileEmitter.lastEmissionTime = SDL_GetTicks64();
    }
};
