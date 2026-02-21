#pragma once

#include <cinttypes>
#include <vector>

namespace Physics {
    void Initialize();
    void Destroy();
    void Update(float deltaTime);
    void Render();

    void Reset();
    using ObjectId = int32_t;

    [[nodiscard]] ObjectId CreateStaticRectangle(float x, float y, float width, float height, bool isTrigger = false, uint8_t filter = 0x01);
    [[nodiscard]] ObjectId CreateStaticCircle(float x, float y, float radius, bool isTrigger = false, uint8_t filter = 0x01);

    [[nodiscard]] ObjectId CreateDynamicRectangle(float x, float y, float width, float height, float mass = 1.f, uint8_t filter = 0x01);
    [[nodiscard]] ObjectId CreateDynamicCircle(float x, float y, float radius, float mass = 1.f, uint8_t filter = 0x01);

    [[nodiscard]] ObjectId CreateKinematicRectangle(float x, float y, float width, float height);

    void AddImpulse(ObjectId objectId, float x, float y);
    void RemoveObject(ObjectId objectId);

    void TraceCircle(ObjectId objectId, float radius);
    void ResolveCollision(ObjectId objectId);

    struct MovableObjectPosition {
        ObjectId id;
        float x;
        float y;
    };

    struct OverlapInfo {
        ObjectId objectId1;
        ObjectId objectId2;

        auto operator<=>(const OverlapInfo&) const = default;
    };

    [[nodiscard]] const std::vector<MovableObjectPosition>& GetMovableObjectPositions();
    [[nodiscard]] const std::vector<OverlapInfo>& GetOverlapInfos();
}
