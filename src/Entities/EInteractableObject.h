#pragma once

#include "EInteractable.h"

namespace World {
    class EInteractableObject : public EInteractable {
    public:
        EInteractableObject(const std::string& name);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;
        void Interact() override;
    };
}
