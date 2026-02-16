#pragma once
#include "EMovable.h"
#include "BoxName.h"

namespace World {
    class EBox : public EMovable {
    public:
        EBox(BoxName boxName) : boxName(boxName) {}
        void OnSpawn() override;
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void Move(float dirX, float dirY, float speed);

        [[nodiscard]] BoxName GetBoxName() const { return boxName; }
    private:
        BoxName boxName;
    };
}
