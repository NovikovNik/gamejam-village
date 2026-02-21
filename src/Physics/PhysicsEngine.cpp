#include "PhysicsEngine.h"

#include <Utils/Singleton.h>
#include <Renderer/Renderer.h>
#include <Game/GameFeatures.h>
#include <Logger/Logger.h>
#include <glm/glm.hpp>
#include <variant>
#include <optional>
#include <set>

struct Rectangle {
    float x{};
    float y{};
    float width{1.f};
    float height{1.f};

    float velocityX{};
    float velocityY{};

    float mass{1.f};

    uint8_t filter{0x01};
};

struct Circle {
    float x{};
    float y{};
    float radius{1.f};

    float velocityX{};
    float velocityY{};

    float mass{1.f};
    
    uint8_t filter{0x01};
};

using PhysicsObject = std::variant<Rectangle, Circle>;

class PhysicsManager : public Singleton<PhysicsManager> {
public:
    void Initialize() {
        Logger::Log("[Physics] Initialized");
    }

    void Destroy() {
        Logger::Log("[Physics] Destroyed");
    }

    void SolveVelocities(float deltaTime) {
        for (auto& object : dynamicObjects) {
            std::visit([deltaTime](auto& obj) {
                obj.x += obj.velocityX * deltaTime;
                obj.y += obj.velocityY * deltaTime;

                obj.velocityX = 0.f;
                obj.velocityY = 0.f;
            }, object);
        }
        for (auto& object : kinematicObjects) {
            std::visit([deltaTime](auto& obj) {
                obj.x += obj.velocityX * deltaTime;
                obj.y += obj.velocityY * deltaTime;

                obj.velocityX = 0.f;
                obj.velocityY = 0.f;
            }, object);
        }
    }

    void PushRectangleFromRectangle(const Rectangle& objectToPush, Rectangle& staticObject) {

    }
    
    void PushRectangleFromCircle(const Rectangle& objectToPush, Circle& staticObject) {

    }

    struct TestResult {
        float penetrationDepth{};
        glm::vec2 penetrationNormal{};
    };

    [[nodiscard]] std::optional<TestResult> TestCircleCircle(const Circle& object1, const Circle& object2) {
        const glm::vec2 object1Pos = glm::vec2{ object1.x, object1.y };
        const glm::vec2 object2Pos = glm::vec2{ object2.x, object2.y };
        const float distance = glm::distance(object1Pos, object2Pos);
        const float radiusSum = object1.radius + object2.radius;
        if (distance > radiusSum) {
            return std::nullopt;
        }

        return TestResult{ radiusSum - distance, glm::normalize(object2Pos - object1Pos) };
    }

    [[nodiscard]] std::optional<TestResult> TestRectangleRectangle(const Rectangle& object1, const Rectangle& object2) {
        const float overlapX = (object1.width  + object2.width)  * 0.5f - std::abs(object1.x - object2.x);
        if (overlapX <= 0.f) return std::nullopt;

        const float overlapY = (object1.height + object2.height) * 0.5f - std::abs(object1.y - object2.y);
        if (overlapY <= 0.f) return std::nullopt;

        // Push along the axis of least penetration
        if (overlapX < overlapY) {
            const float signX = object2.x >= object1.x ? 1.f : -1.f;
            return TestResult{ overlapX, glm::vec2{ signX, 0.f } };
        } else {
            const float signY = object2.y >= object1.y ? 1.f : -1.f;
            return TestResult{ overlapY, glm::vec2{ 0.f, signY } };
        }
    }
    
    [[nodiscard]] std::optional<TestResult> TestRectangleCircle(const Rectangle& object1, const Circle& object2) {
        // Closest point on the AABB to the circle centre
        const float closestX = std::clamp(object2.x, object1.x - object1.width  * 0.5f, object1.x + object1.width  * 0.5f);
        const float closestY = std::clamp(object2.y, object1.y - object1.height * 0.5f, object1.y + object1.height * 0.5f);

        const glm::vec2 diff{ object2.x - closestX, object2.y - closestY };
        const float distSq = glm::dot(diff, diff);

        if (distSq >= object2.radius * object2.radius) return std::nullopt;

        const float dist = std::sqrt(distSq);
        if (dist < 1e-6f) {
            // Circle centre is inside the rectangle — push out along nearest axis
            const float overlapX = object1.width  * 0.5f + object2.radius - std::abs(object2.x - object1.x);
            const float overlapY = object1.height * 0.5f + object2.radius - std::abs(object2.y - object1.y);
            if (overlapX < overlapY) {
                const float signX = object2.x >= object1.x ? 1.f : -1.f;
                return TestResult{ overlapX, glm::vec2{ signX, 0.f } };
            } else {
                const float signY = object2.y >= object1.y ? 1.f : -1.f;
                return TestResult{ overlapY, glm::vec2{ 0.f, signY } };
            }
        }

        return TestResult{ object2.radius - dist, diff / dist };
    }

    template <typename T1, typename T2>
    void Push(
        const std::optional<TestResult>& result,
        const Physics::OverlapInfo& overlapInfo,
        bool justTest,
        T1& object1,
        T2& object2,
        std::set<Physics::OverlapInfo>& overlapInfosSet
    )
    {
        if (!result) {
            return;
        }
        overlapInfosSet.insert(overlapInfo);
        if (justTest) {
            return;
        }
        if (object1.mass == 0.f) {
            object2.x += result->penetrationNormal.x * result->penetrationDepth;
            object2.y += result->penetrationNormal.y * result->penetrationDepth;
            return;
        }
        else if (object2.mass == 0.f) {
            object1.x -= result->penetrationNormal.x * result->penetrationDepth;
            object1.y -= result->penetrationNormal.y * result->penetrationDepth;
            return;
        }
        const float mass1 = object1.mass;
        const float mass2 = object2.mass;
        const float totalMass = mass1 + mass2;
        const float ratio1 = mass2 / totalMass;
        const float ratio2 = mass1 / totalMass;
        object1.x -= result->penetrationNormal.x * result->penetrationDepth * ratio1;
        object1.y -= result->penetrationNormal.y * result->penetrationDepth * ratio1;
        object2.x += result->penetrationNormal.x * result->penetrationDepth * ratio2;
        object2.y += result->penetrationNormal.y * result->penetrationDepth * ratio2;
    }

    void SolveConstraints(
        float deltaTime, 
        std::vector<PhysicsObject>& objects1,
        const std::vector<Physics::ObjectId>& objects1Ids,
        std::vector<PhysicsObject>& objects2,
        const std::vector<Physics::ObjectId>& objects2Ids,
        std::set<Physics::OverlapInfo>& overlapInfosSet, 
        bool justTest
    ) {
        
        for (size_t object1Index = 0; object1Index < objects1.size(); ++object1Index) {
            auto& object1 = objects1[object1Index];
            std::visit([this, justTest, &objects2, &objects1Ids, &objects2Ids, object1Index, &overlapInfosSet](auto& obj1) {
                for (size_t object2Index = 0; object2Index < objects2.size(); ++object2Index) {
                    auto& object2 = objects2[object2Index];
                    const Physics::OverlapInfo overlapInfo = {
                        .objectId1 = objects1Ids[object1Index],
                        .objectId2 = objects2Ids[object2Index],
                    };
                    std::visit([this, &obj1, justTest, &overlapInfo, &overlapInfosSet](auto& obj2) {
                        if ((obj1.filter & obj2.filter) == 0) {
                            return;
                        }
                        using TObject1 = std::decay_t<decltype(obj1)>;
                        using TObject2 = std::decay_t<decltype(obj2)>;
                        if constexpr (std::is_same_v<TObject1, Rectangle> && std::is_same_v<TObject2, Rectangle>) {
                            const auto result = TestRectangleRectangle(obj1, obj2);
                            Push(result, overlapInfo, justTest, obj1, obj2, overlapInfosSet);
                        }
                        else if constexpr (std::is_same_v<TObject1, Rectangle> && std::is_same_v<TObject2, Circle>) {
                            const auto result = TestRectangleCircle(obj1, obj2);
                            Push(result, overlapInfo, justTest, obj1, obj2, overlapInfosSet);
                        }
                        else if constexpr (std::is_same_v<TObject1, Circle> && std::is_same_v<TObject2, Rectangle>) {
                            const auto result = TestRectangleCircle(obj2, obj1);
                            Push(result, overlapInfo, justTest, obj1, obj2, overlapInfosSet);
                        }
                        else if constexpr (std::is_same_v<TObject1, Circle> && std::is_same_v<TObject2, Circle>) {
                            const auto result = TestCircleCircle(obj1, obj2);
                            Push(result, overlapInfo, justTest, obj1, obj2, overlapInfosSet);
                        }
                    }, object2);
                }
            }, object1);
        }
    }
    
    void SolveConstraintsSameType(
        float deltaTime,
        std::vector<PhysicsObject>& objects,
        const std::vector<Physics::ObjectId>& objectsIds,
        std::set<Physics::OverlapInfo>& overlapInfosSet,
        bool justTest
    ) {
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                auto& object1 = objects[i];
                auto& object2 = objects[j];
                const Physics::OverlapInfo overlapInfo = {
                    .objectId1 = objectsIds[i],
                    .objectId2 = objectsIds[j],
                };
                std::visit([&](auto& object1, auto& object2) {
                    if ((object1.filter & object2.filter) == 0) {
                        return;
                    }
                    using TObject1 = std::decay_t<decltype(object1)>;
                    using TObject2 = std::decay_t<decltype(object2)>;
                    if constexpr (std::is_same_v<TObject1, Rectangle> && std::is_same_v<TObject2, Rectangle>) {
                        const auto result = TestRectangleRectangle(object1, object2);
                        Push(result, overlapInfo, justTest, object1, object2, overlapInfosSet);
                    }
                    else if constexpr (std::is_same_v<TObject1, Rectangle> && std::is_same_v<TObject2, Circle>) {
                        const auto result = TestRectangleCircle(object1, object2);
                        Push(result, overlapInfo, justTest, object1, object2, overlapInfosSet);
                    }
                    else if constexpr (std::is_same_v<TObject1, Circle> && std::is_same_v<TObject2, Rectangle>) {
                        const auto result = TestRectangleCircle(object2, object1);
                        Push(result, overlapInfo, justTest, object1, object2, overlapInfosSet);
                    }
                    else if constexpr (std::is_same_v<TObject1, Circle> && std::is_same_v<TObject2, Circle>) {
                        const auto result = TestCircleCircle(object1, object2);
                        Push(result, overlapInfo, justTest, object1, object2, overlapInfosSet);
                    }
                }, object1, object2);
            }
        }
    }

    void SolveConstraints(float deltaTime) {
        constexpr int numOfIterations = 10;
        overlapInfos.clear();
        std::set<Physics::OverlapInfo> overlapInfosSet;

        for (int i = 0; i < numOfIterations; ++i) {
            float timeStep = deltaTime / numOfIterations;
            SolveConstraints(deltaTime, dynamicObjects, dynamicObjectsIds, triggerObjects, triggerObjectsIds, overlapInfosSet, true);
            SolveConstraints(deltaTime, dynamicObjects, dynamicObjectsIds, staticObjects, staticObjectsIds, overlapInfosSet, false);
            SolveConstraintsSameType(deltaTime, dynamicObjects, dynamicObjectsIds, overlapInfosSet, false);
        }
        overlapInfos.assign(overlapInfosSet.begin(), overlapInfosSet.end());
    }

    void CashDynamicObjectPositions() {
        movableObjectPositions.clear();

        for (size_t i = 0; i < kinematicObjectsIds.size(); ++i) {
            const auto id = kinematicObjectsIds[i];
            const auto& object = kinematicObjects[i];
            std::visit([&](auto& obj) {
                movableObjectPositions.push_back({ id, obj.x, obj.y });
            }, object);
        }

        for (size_t i = 0; i < dynamicObjectsIds.size(); ++i) {
            const auto id = dynamicObjectsIds[i];
            const auto& object = dynamicObjects[i];
            std::visit([&](auto& obj) {
                movableObjectPositions.push_back({ id, obj.x, obj.y });
            }, object);
        }
    }

    void Update(float deltaTime) {
        SolveVelocities(deltaTime);
        SolveConstraints(deltaTime);

        CashDynamicObjectPositions();
    }

    void RenderObjects(const std::vector<PhysicsObject>& objects) {
        for (const auto& object : objects) {
            std::visit([&](auto& obj) {
                using T = std::decay_t<decltype(obj)>;
                if constexpr (std::is_same_v<T, Rectangle>) {
                    Renderer::DrawRectangle(obj.x, obj.y, obj.width, obj.height, 0.0);
                } else if constexpr (std::is_same_v<T, Circle>) {
                    Renderer::DrawCircle(obj.x, obj.y, obj.radius);
                }
            }, object);
        }
    }

    void Render() {
        if (GameFeatures::isDebug) { 
            RenderObjects(staticObjects);
            RenderObjects(dynamicObjects);
            RenderObjects(kinematicObjects);
            RenderObjects(triggerObjects);
        }
    }

    void Reset() {
        // Logger::Log("[Physics] Reset");

        staticObjects.clear();
        dynamicObjects.clear();
        staticObjectsIds.clear();
        dynamicObjectsIds.clear();
        kinematicObjects.clear();
        kinematicObjectsIds.clear();
        movableObjectPositions.clear();
        triggerObjects.clear();
        triggerObjectsIds.clear();
    }

    Physics::ObjectId CreateStaticRectangle(float x, float y, float width, float height, bool isTrigger, uint8_t filter) {
        if (isTrigger) {
            triggerObjects.push_back(Rectangle{ 
                .x = x, 
                .y = y, 
                .width = width, 
                .height = height,
                .mass = 0.f,
                .filter = filter
            });
            triggerObjectsIds.push_back(++lastObjectId);
            return lastObjectId;
        }
        staticObjects.push_back(Rectangle{ 
            .x = x, 
            .y = y, 
            .width = width, 
            .height = height,
            .mass = 0.f,
            .filter = filter
        });
        staticObjectsIds.push_back(++lastObjectId);
        return lastObjectId;
    }

    Physics::ObjectId CreateStaticCircle(float x, float y, float radius, bool isTrigger, uint8_t filter) {
        if (isTrigger) {
            triggerObjects.push_back(Circle{ 
                .x = x, 
                .y = y, 
                .radius = radius,
                .mass = 0.f,
                .filter = filter
            });
            triggerObjectsIds.push_back(++lastObjectId);
            return lastObjectId;
        }
        staticObjects.push_back(Circle{ 
            .x = x, 
            .y = y, 
            .radius = radius,
            .mass = 0.f,
            .filter = filter
        });
        staticObjectsIds.push_back(++lastObjectId);
        return lastObjectId;
    }

    Physics::ObjectId CreateDynamicRectangle(float x, float y, float width, float height, float mass, uint8_t filter) {
        dynamicObjects.push_back(Rectangle{ 
            .x = x, 
            .y = y, 
            .width = width, 
            .height = height, 
            .mass = mass,
            .filter = filter
        });
        dynamicObjectsIds.push_back(++lastObjectId);
        return lastObjectId;
    }

    Physics::ObjectId CreateDynamicCircle(float x, float y, float radius, float mass, uint8_t filter) {
        dynamicObjects.push_back(Circle{ 
            .x = x, 
            .y = y, 
            .radius = radius, 
            .mass = mass,
            .filter = filter
        });
        dynamicObjectsIds.push_back(++lastObjectId);
        return lastObjectId;
    }

    Physics::ObjectId CreateKinematicRectangle(float x, float y, float width, float height) {
        kinematicObjects.push_back(Rectangle{ x, y, width, height });
        kinematicObjectsIds.push_back(++lastObjectId);
        return lastObjectId;
    }

    void AddImpulse(Physics::ObjectId objectId, float x, float y) {
        auto it = std::find(dynamicObjectsIds.begin(), dynamicObjectsIds.end(), objectId);
        if (it == dynamicObjectsIds.end()) {
            return;
        }

        const auto index = std::distance(dynamicObjectsIds.begin(), it);
        std::visit([x, y](auto& obj) {
            obj.velocityX += x;
            obj.velocityY += y;
        }, dynamicObjects[index]);
    }

    [[nodiscard]] bool TryRemoveDynamicObject(Physics::ObjectId objectId) {
        auto it = std::find(dynamicObjectsIds.begin(), dynamicObjectsIds.end(), objectId);
        if (it == dynamicObjectsIds.end()) {
            return false;
        }
        const auto index = std::distance(dynamicObjectsIds.begin(), it);
        dynamicObjects.erase(dynamicObjects.begin() + index);
        dynamicObjectsIds.erase(dynamicObjectsIds.begin() + index);
        return true;
    }

    [[nodiscard]] bool TryRemoveStaticObject(Physics::ObjectId objectId) {
        auto it = std::find(staticObjectsIds.begin(), staticObjectsIds.end(), objectId);
        if (it == staticObjectsIds.end()) {
            return false;
        }
        const auto index = std::distance(staticObjectsIds.begin(), it);
        staticObjects.erase(staticObjects.begin() + index);
        staticObjectsIds.erase(staticObjectsIds.begin() + index);
        return true;
        
    }

    [[nodiscard]] bool TryRemoveTriggerObject(Physics::ObjectId objectId) {
        auto it = std::find(triggerObjectsIds.begin(), triggerObjectsIds.end(), objectId);
        if (it == triggerObjectsIds.end()) {
            return false;
        }
        const auto index = std::distance(triggerObjectsIds.begin(), it);
        triggerObjects.erase(triggerObjects.begin() + index);
        triggerObjectsIds.erase(triggerObjectsIds.begin() + index);
        return true;
        
    }

    void RemoveObject(Physics::ObjectId objectId) {
        if (TryRemoveDynamicObject(objectId)) {
            return;
        }
        if (TryRemoveStaticObject(objectId)) {
            return;
        }
        if (TryRemoveTriggerObject(objectId)) {
            return;
        }
    }

    void ResolveCollision(Physics::ObjectId objectId) {
        // Logger::Log("[Physics] Resolved collision");
    }

    void TraceCircle(Physics::ObjectId objectId, float radius) {
        // Logger::Log("[Physics] Traced circle");
    }

    const std::vector<Physics::MovableObjectPosition>& GetMovableObjectPositions() {
        return movableObjectPositions;
    }

    const std::vector<Physics::OverlapInfo>& GetOverlapInfos() {
        return overlapInfos;
    }

private:
    std::vector<PhysicsObject> staticObjects;
    std::vector<Physics::ObjectId> staticObjectsIds;
    
    std::vector<PhysicsObject> triggerObjects;
    std::vector<Physics::ObjectId> triggerObjectsIds;

    std::vector<PhysicsObject> dynamicObjects;
    std::vector<Physics::ObjectId> dynamicObjectsIds;

    std::vector<PhysicsObject> kinematicObjects;
    std::vector<Physics::ObjectId> kinematicObjectsIds;

    Physics::ObjectId lastObjectId = 0;

    std::vector<Physics::MovableObjectPosition> movableObjectPositions;

    std::vector<Physics::OverlapInfo> overlapInfos;
};

namespace Physics {
    void Initialize() {
        PhysicsManager::instance().Initialize();
    }

    void Destroy() {
        PhysicsManager::instance().Destroy();
    }

    void Update(float deltaTime) {
        PhysicsManager::instance().Update(deltaTime);
    }

    void Render() {
        PhysicsManager::instance().Render();
    }

    void Reset() {
        PhysicsManager::instance().Reset();
    }

    ObjectId CreateStaticRectangle(float x, float y, float width, float height, bool isTrigger, uint8_t filter) {
        return PhysicsManager::instance().CreateStaticRectangle(x, y, width, height, isTrigger, filter);
    }

    ObjectId CreateStaticCircle(float x, float y, float radius, bool isTrigger, uint8_t filter) {
        return PhysicsManager::instance().CreateStaticCircle(x, y, radius, isTrigger, filter);
    }

    ObjectId CreateDynamicRectangle(float x, float y, float width, float height, float mass, uint8_t filter) {
        return PhysicsManager::instance().CreateDynamicRectangle(x, y, width, height, mass, filter);
    }

    ObjectId CreateDynamicCircle(float x, float y, float radius, float mass, uint8_t filter) {
        return PhysicsManager::instance().CreateDynamicCircle(x, y, radius, mass, filter);
    }

    ObjectId CreateKinematicRectangle(float x, float y, float width, float height) {
        return PhysicsManager::instance().CreateKinematicRectangle(x, y, width, height);
    }

    void AddImpulse(ObjectId objectId, float x, float y) {
        PhysicsManager::instance().AddImpulse(objectId, x, y);
    }

    void RemoveObject(ObjectId objectId) {
        PhysicsManager::instance().RemoveObject(objectId);
    }

    void ResolveCollision(ObjectId objectId) {
        PhysicsManager::instance().ResolveCollision(objectId);
    }

    void TraceCircle(ObjectId objectId, float radius) {
        PhysicsManager::instance().TraceCircle(objectId, radius);
    }

    const std::vector<MovableObjectPosition>& GetMovableObjectPositions() {
        return PhysicsManager::instance().GetMovableObjectPositions();
    }

    const std::vector<OverlapInfo>& GetOverlapInfos() {
        return PhysicsManager::instance().GetOverlapInfos();
    }
}
