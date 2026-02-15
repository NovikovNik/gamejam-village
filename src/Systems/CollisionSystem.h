#pragma once

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class CollisionSystem: public System {
    public:
        CollisionSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<BoxColliderComponent>();
        }

        void Update(std::unique_ptr<EventBus>& eventBus) {
            auto entities = GetSystemEntities(); // важно: один раз получить список

            for (auto e : entities) {
                e.GetComponent<BoxColliderComponent>().debugColor = {255, 0, 0, 255};
            }

            for (size_t i = 0; i < entities.size(); i++) {
                for (size_t j = i + 1; j < entities.size(); j++) {

                    Entity a = entities[i];
                    Entity b = entities[j];

                    if (CheckCollisionAABB(a, b)) {
                        auto& aCol = a.GetComponent<BoxColliderComponent>();
                        auto& bCol = b.GetComponent<BoxColliderComponent>();
                        aCol.debugColor = {0, 255, 0, 255};
                        bCol.debugColor = {0, 255, 0, 255};

                        eventBus->EmitEvent<CollisionEvent>(a, b);
                    }
                }
            }
        }

        bool CheckCollisionAABB(Entity& entity, Entity& otherEntity) {
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& boxCollider = entity.GetComponent<BoxColliderComponent>();
            auto& otherTransform = otherEntity.GetComponent<TransformComponent>();
            auto& otherBoxCollider = otherEntity.GetComponent<BoxColliderComponent>();

            int aWidth = boxCollider.width * transform.scale.x;
            int aHeight = boxCollider.height * transform.scale.y;
            int aOffsetX = boxCollider.offset.x * transform.scale.x;
            int aOffsetY = boxCollider.offset.y * transform.scale.y;
            int bWidth = otherBoxCollider.width * otherTransform.scale.x;
            int bHeight = otherBoxCollider.height * otherTransform.scale.y;
            int bOffsetX = otherBoxCollider.offset.x * otherTransform.scale.x;
            int bOffsetY = otherBoxCollider.offset.y * otherTransform.scale.y;

            return (transform.position.x + aOffsetX < otherTransform.position.x + bOffsetX + bWidth &&
                    transform.position.x + aOffsetX + aWidth > otherTransform.position.x + bOffsetX &&
                    transform.position.y + aOffsetY < otherTransform.position.y + bOffsetY + bHeight &&
                    transform.position.y + aOffsetY + aHeight > otherTransform.position.y + bOffsetY);
        }
};
