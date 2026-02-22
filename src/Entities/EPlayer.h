#pragma once

#include "EMovable.h"
#include <glm/glm.hpp>
#include <Events/InterectButtonPressedEvent.h>
#include <Renderer/Renderer.h>
#include <Physics/PhysicsEngine.h>

namespace World {
    class EPlayer : public EMovable {
    public:
        EPlayer(const std::string& name);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void OnSpawn(float x, float y, float w, float h) override;
        void SetPosition(float x, float y) override;
        void OnMoved(float deltaTime);

        virtual void SetTooltipTexture(Renderer::TextureId texture, float w, float h);
        virtual void OnInterectButtonPressed(::InterectButtonPressedEvent& event);

        [[nodiscard]] class EInteractable* TryInteract() const;

    private:
        // Pixels per frame

        glm::vec2 direction; // for normalization kinda
        bool horizontalFlip = false;
        float basicSpeed = 140.f;
        std::string name;
        bool bShowTooltip = false;

        float footstepTimer = 0;  // start at max → first step plays immediately
        Renderer::TextureId tooltipTexture;
        float tooltipWidth = 32.0f;
        float tooltipHeight = 32.0f;

        Events::Handler onInterectButtonPressed;
        Renderer::AnimationHandle animationIdle;
        Renderer::AnimationHandle animationMoving;

        class EInteractable* currentInteractable = nullptr;

        Physics::ObjectId physicsObjectId{};
    };
}
