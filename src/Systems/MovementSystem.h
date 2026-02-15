#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "SDL_render.h"
#include <algorithm>

class MovementSystem: public System {
    public:
        MovementSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();
        }

        void SubscribeToEvents(const std::unique_ptr<EventBus>& eventBus) {
            eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollisionEvent);
        }

        void onCollisionEvent(CollisionEvent& event) {
            Logger::Log("CollisionEvent received: " + std::to_string(event.entityA.GetId()) + " and " + std::to_string(event.entityB.GetId()));
            auto a = event.entityA;
            auto b = event.entityB;

            if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
                onEnemyHitsObstacle(a, b);
            }

            if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
                onEnemyHitsObstacle(b, a);
            }
        }

        void onEnemyHitsObstacle(Entity& a, Entity& b) {
            if (a.HasComponent<RigidBodyComponent>() && a.HasComponent<SpriteComponent>()) {
                auto& rigidBody = a.GetComponent<RigidBodyComponent>();
                auto& spriteComponent = a.GetComponent<SpriteComponent>();

                if (rigidBody.velocity.x != 0) {
                    rigidBody.velocity.x *= -1;
                    spriteComponent.flip = spriteComponent.flip == SDL_FLIP_NONE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                }
                if (rigidBody.velocity.y != 0) {
                    rigidBody.velocity.y *= -1;
                    spriteComponent.flip = spriteComponent.flip == SDL_FLIP_NONE ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
                }
            }
        }

        void Update(double deltaTime) {
            for (auto entity: GetSystemEntities()) {
                auto& transform = entity.GetComponent<TransformComponent>();
                const auto& sprite = entity.GetComponent<SpriteComponent>();
                const auto rigidBody = entity.GetComponent<RigidBodyComponent>();

                bool isEntityOutsideTheMap = (
                    transform.position.x + sprite.width < 0 ||
                    transform.position.x > Game::mapWidth ||
                    transform.position.y + sprite.height < 0 ||
                    transform.position.y > Game::mapHeight
                );

                transform.position.x += rigidBody.velocity.x * deltaTime;
                transform.position.y += rigidBody.velocity.y * deltaTime;

                if (entity.HasTag("Player")) {
                    transform.position.x = std::clamp(transform.position.x, 0.0f, static_cast<float>(Game::mapWidth) - static_cast<float>(sprite.width));
                    transform.position.y = std::clamp(transform.position.y, 0.0f, static_cast<float>(Game::mapHeight) - 50.0f);
                }

                // Killing enemies outside the map
                if (isEntityOutsideTheMap && !entity.HasTag("Player")) {
                    entity.Kill();
                }
            //     Logger::Log("[Entity " + std::to_string(entity.GetId()) + "] Transform position: " + std::to_string(transform.position.x) + ", " + std::to_string(transform.position.y));
            }
        }
};
