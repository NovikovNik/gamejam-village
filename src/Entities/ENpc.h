#pragma once

#include "EInteractable.h"

namespace World {
    class ENpc : public EInteractable {
    public:
        ENpc(const std::string& name, float x, float y);
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void Interact() override;

    private:
        std::string name;
    };
}
