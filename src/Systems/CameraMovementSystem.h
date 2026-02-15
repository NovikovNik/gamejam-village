#pragma once
#include "../ECS/ECS.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include "SDL_rect.h"

class CameraMovementSystem: public System {
    public:
        CameraMovementSystem() {
            RequireComponent<CameraFollowComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                auto transform = entity.GetComponent<TransformComponent>();

                camera.x = (int)(transform.position.x - camera.w / 2);
                camera.y = (int)(transform.position.y - camera.h / 2);

                int maxX = (int)Game::mapWidth  - camera.w;
                int maxY = (int)Game::mapHeight - camera.h;

                if (maxX < 0) maxX = 0;
                if (maxY < 0) maxY = 0;

                if (camera.x < 0) camera.x = 0;
                if (camera.y < 0) camera.y = 0;
                if (camera.x > maxX) camera.x = maxX;
                if (camera.y > maxY) camera.y = maxY;
            }
        }
};
