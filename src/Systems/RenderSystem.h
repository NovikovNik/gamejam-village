#pragma once

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../AssetManager/AssetManager.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include <SDL.h>

class RenderSystem: public System {
    public:
        RenderSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<SpriteComponent>();
        }

        void Update(SDL_Renderer* renderer, AssetManager& assetManager, SDL_Rect& camera) {
            struct RenderableEntity {
                TransformComponent transformComponent;
                SpriteComponent spriteComponent;
            };

            std::vector<RenderableEntity> renderableEntities;

            for (auto entity: GetSystemEntities()) {
                RenderableEntity renderableEntity;
                renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();
                renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();

                // Bypass rendering entities if they are outside the camera view
                bool isEntityOustideCameraView = (
                    renderableEntity.transformComponent.position.x + (renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width) < camera.x ||
                    renderableEntity.transformComponent.position.x > camera.x + camera.w ||
                    renderableEntity.transformComponent.position.y + (renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height) < camera.y ||
                    renderableEntity.transformComponent.position.y > camera.y + camera.h
                );

                if (isEntityOustideCameraView && !renderableEntity.spriteComponent.isFixed) {
                    continue;
                }
                renderableEntities.emplace_back(renderableEntity);
            };

            std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
                return a.spriteComponent.zindex < b.spriteComponent.zindex;
            });

            for (auto renderableEntity: renderableEntities) {
                const auto& transform = renderableEntity.transformComponent;
                const auto& sprite = renderableEntity.spriteComponent;

                SDL_Rect srcRect = sprite.srcRect;
                SDL_Rect destRect = {
                    static_cast<int>(transform.position.x - (!sprite.isFixed ? camera.x : 0)),
                    static_cast<int>(transform.position.y - (!sprite.isFixed ? camera.y : 0)),
                    static_cast<int>(sprite.width * transform.scale.x),
                    static_cast<int>(sprite.height * transform.scale.y)
                };
                SDL_RenderCopyEx(
                    renderer,
                    assetManager.GetTexture(sprite.assetId),
                    &srcRect,
                    &destRect,
                    transform.rotation,
                    NULL,
                    sprite.flip
                );
            }
        }
};
