#pragma once
#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectTileComponent.h"
#include "../Components/HealthComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include <string>

class DamageSystem: public System {
    public:
        DamageSystem() {
            RequireComponent<BoxColliderComponent>();
        }

        void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
            eventBus->SubscribeToEvent<CollisionEvent, DamageSystem>(this, &DamageSystem::onCollisionEvent);
        }

        void onCollisionEvent(CollisionEvent& event) {
            Logger::Log("CollisionEvent received: " + std::to_string(event.entityA.GetId()) + " and " + std::to_string(event.entityB.GetId()));
            auto a = event.entityA;
            auto b = event.entityB;

            if (a.BelongsToGroup("projectiles") && b.HasTag("Player")) {
                onProjectileHitsPlayer(a, b);
            }

            if (b.BelongsToGroup("projectiles") && a.HasTag("Player")) {
                onProjectileHitsPlayer(b, a);
            }

            if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
                onProjectileHitsEnemy(a, b);
            }

            if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
                onProjectileHitsEnemy(b, a);
            }
        }

        void onProjectileHitsPlayer(Entity& projectile, Entity& player) {
            auto projectileComponent = projectile.GetComponent<ProjectTileComponent>();
            if (!projectileComponent.isFriendly) {
                auto& health = player.GetComponent<HealthComponent>();
                health.health -= projectileComponent.hitPercentDamage;
                Logger::Log("Player health: " + std::to_string(health.health));
                if (health.health <= 0) {
                    player.Kill();
                }
                projectile.Kill();
            }
        }

        void onProjectileHitsEnemy(Entity& projectile, Entity& enemy) {
            auto projectileComponent = projectile.GetComponent<ProjectTileComponent>();
            if (projectileComponent.isFriendly) {
                auto& health = enemy.GetComponent<HealthComponent>();
                health.health -= projectileComponent.hitPercentDamage;
                Logger::Log("Player health: " + std::to_string(health.health));
                if (health.health <= 0) {
                    enemy.Kill();
                }
                projectile.Kill();
            }
        }

        void Update(std::unique_ptr<EventBus>& eventBus) {
        }
};
