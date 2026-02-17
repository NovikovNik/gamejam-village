#pragma once

#include "EMapObject.h"
#include "InteractName.h"

namespace World {
    class EInteractable : public EMapObject {
    public:
        EInteractable(InteractId interactId) : interactId(interactId) {}
        bool Update(float deltaTime) override;

        virtual void Interact();

        [[nodiscard]] InteractId GetInteractId() const { return interactId; }

    private:
        InteractId interactId;
    };
}