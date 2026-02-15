#pragma once

#include "../ECS/ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "SDL_rect.h"
#include <SDL.h>

class RenderColliderSystem: public System {
    public:
        RenderColliderSystem() {
            RequireComponent<BoxColliderComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Renderer* renderer, SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                auto& boxCollider = entity.GetComponent<BoxColliderComponent>();
                auto& transform = entity.GetComponent<TransformComponent>();
                int width = boxCollider.width * transform.scale.x;
                int height = boxCollider.height * transform.scale.y;
                int offsetX = boxCollider.offset.x * transform.scale.x;
                int offsetY = boxCollider.offset.y * transform.scale.y;
                SDL_Rect rect = {
                    static_cast<int>(transform.position.x + offsetX - camera.x),
                    static_cast<int>(transform.position.y + offsetY - camera.y),
                    width,
                    height
                };
                SDL_SetRenderDrawColor(renderer, boxCollider.debugColor.r, boxCollider.debugColor.g, boxCollider.debugColor.b, boxCollider.debugColor.a);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
};
