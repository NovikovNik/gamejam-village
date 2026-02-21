#pragma once

#include "EMapObject.h"
#include "InteractName.h"
#include <Physics/PhysicsEngine.h>

namespace World {
    class EInteractable : public EMapObject {
    public:
        EInteractable(InteractId interactId) : interactId(interactId) {}
        bool Update(float deltaTime) override;

        virtual void Interact();

        [[nodiscard]] InteractId GetInteractId() const { return interactId; }
        virtual void CreatePhysicsObjects();
        virtual void RemovePhysicsObjects();

        virtual void OnDestroy();

        void OnSpawn(float x, float y, float w, float h) override;

    protected:
        InteractId interactId;
        Physics::ObjectId physicsTriggerId{};
    };
}