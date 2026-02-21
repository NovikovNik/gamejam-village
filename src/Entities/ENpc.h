#pragma once

#include "EInteractable.h"
#include <Physics/PhysicsEngine.h>
#include <Renderer/Renderer.h>

namespace World {
    class ENpc : public EInteractable {
    public:
        ENpc(const std::string& name);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void Interact() override;
        void SetHorizontalFlip(bool flip) { horizontalFlip = flip; }

        void OnSpawn(float x, float y, float w, float h) override;

        void CreatePhysicsObjects() override;
        void RemovePhysicsObjects() override;

    private:
        bool horizontalFlip = false;
        std::string name;

        Physics::ObjectId physicsColliderId;

        Renderer::AnimationHandle animationIdle;
    };
}
