#pragma once

#include "../ECS/ECS.h"
#include "../Components/HealthComponent.h"
#include "../Components/HealthBarComponent.h"
#include "../Components/TransformComponent.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include <SDL.h>
#include <string>

namespace Colors {
    static constexpr SDL_Color white = {255, 255, 255};
    static constexpr SDL_Color green = {0, 255, 0};
    static constexpr SDL_Color yellow = {255, 255, 0};
    static constexpr SDL_Color red = {255, 0, 0};
};

class HealthBarSystem: public System {
    public:
        HealthBarSystem() {
            RequireComponent<HealthComponent>();
            RequireComponent<HealthBarComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Renderer* renderer, AssetManager& assetManager, SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                auto& healthComponent = entity.GetComponent<HealthComponent>();
                auto& healthBarComponent = entity.GetComponent<HealthBarComponent>();
                auto& transformComponent = entity.GetComponent<TransformComponent>();

                if (healthComponent.health >= 60) {
                    healthBarComponent.color = Colors::green;
                } else if (healthComponent.health < 60 && healthComponent.health >= 30) {
                    healthBarComponent.color = Colors::yellow;
                } else {
                    healthBarComponent.color = Colors::red;
                }

                std::string hpText = std::to_string(healthComponent.health) + "%";
                SDL_Surface* surface = TTF_RenderText_Blended(
                assetManager.GetFont("pico8-hp"),
                hpText.c_str(),
                healthBarComponent.color);

                SDL_Rect rect = {
                    static_cast<int>(transformComponent.position.x - camera.x + 36),
                    static_cast<int>(transformComponent.position.y - camera.y),
                    healthBarComponent.width * healthComponent.health / 100,
                    healthBarComponent.height
                };
                SDL_SetRenderDrawColor(renderer,
                    healthBarComponent.color.r,
                    healthBarComponent.color.g,
                    healthBarComponent.color.b,
                    healthBarComponent.color.a);
                SDL_RenderFillRect(renderer, &rect);

                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                int labelWidth = 0;
                int labelHeight = 0;
                SDL_QueryTexture(texture, NULL, NULL, &labelWidth, &labelHeight);
                SDL_Rect dstRect = {
                    static_cast<int>(rect.x),
                    static_cast<int>(rect.y + 10),
                    labelWidth,
                    labelHeight
                };
                SDL_RenderCopy(renderer, texture, NULL, &dstRect);
                SDL_DestroyTexture(texture);
            }
        }
};
