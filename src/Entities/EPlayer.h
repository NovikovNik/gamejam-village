#pragma once

#include "EMovable.h"
#include <glm/glm.hpp>
#include <Events/InterectButtonPressedEvent.h>

namespace World {
    class EPlayer : public EMovable {
    public:
        EPlayer(const std::string& name, float x, float y);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void OnSpawn() override;
        void OnMoved(float deltaTime);

        virtual void SetTooltipTexture(Renderer::TextureId texture, float w, float h);
        virtual void OnInterectButtonPressed(::InterectButtonPressedEvent& event);

        [[nodiscard]] class EInteractable* TryInteract() const;

    private:
        // Pixels per frame

        glm::vec2 direction; // for normalization kinda
        float basicSpeed = 140.0f;
        std::string name;
        bool bShowTooltip = false;
        Renderer::TextureId tooltipTexture;
        float tooltipWidth = 32.0f;
        float tooltipHeight = 32.0f;

        Events::Handler onInterectButtonPressed;

        class EInteractable* currentInteractable = nullptr;
    };
}
