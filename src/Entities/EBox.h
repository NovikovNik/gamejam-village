#pragma once
#include "EMovable.h"
#include "BoxName.h"
#include <Physics/PhysicsEngine.h>
#include <Events/PitEvent.h>

namespace World {
    class EBox : public EMovable {
    public:
        EBox(BoxName boxName, float mass = 1.0f) : boxName(boxName), mass(mass) {}
        void OnSpawn(float x, float y, float w, float h) override;
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void OnDestroy() override;

        void Move(float dirX, float dirY, float speed, float deltaTime);
        // Возможно здесь более правильно было бы CanNoveX и CanMoveY, чтобы не блокировать игрока по второй оси
        [[nodiscard]] bool CanMove(float dirX, float dirY, float speed, float deltaTime) const;

        [[nodiscard]] BoxName GetBoxName() const { return boxName; }


        void OnPitBoxOverlap(PitBoxOverlapEvent& event);
    private:
        BoxName boxName;
        float mass{};
        Physics::ObjectId physicsObjectId{};
        Events::Handler pitBoxOverlapHandler{};
    };
}
