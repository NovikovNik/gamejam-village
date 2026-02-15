#pragma once
#include "../ECS/ECS.h"
#include "../Components/ProjectTileComponent.h"
#include "SDL_timer.h"

class ProjectTileLifeCycleSystem : public System {
    public:
    ProjectTileLifeCycleSystem() {
        RequireComponent<ProjectTileComponent>();
    }

    void Update() {
        for (auto entity: GetSystemEntities()) {
            auto& projectTile = entity.GetComponent<ProjectTileComponent>();

            if (SDL_GetTicks64() - projectTile.startTime > projectTile.duration) {
                entity.Kill();
            }
        }
    }
};
