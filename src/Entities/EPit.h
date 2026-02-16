#pragma once
#include "EMovable.h"
#include "BoxName.h"

namespace World {
    class EPit : public EMovable {
    public:
        EPit(BoxName matchBoxName) : matchBoxName(matchBoxName) {}
        void OnSpawn() override;
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        [[nodiscard]] bool IsBoxNameMatch(BoxName boxName) const;
    private:
        BoxName matchBoxName;
    };
}
