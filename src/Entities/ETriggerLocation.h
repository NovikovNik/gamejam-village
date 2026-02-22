#pragma once
#include <Renderer/Renderer.h>
#include <string>
#include <Physics/PhysicsEngine.h>
#include "EMapObject.h"

namespace World {
    class ETriggerLocation : public EMapObject {
    public:
        ETriggerLocation(const std::string& locationName, const std::string& spawnPointMatch, bool tease) : locationName(locationName), spawnPointMatch(spawnPointMatch), tease(tease) {}
        void Render(float deltaTime) override;
        void ChangeLocation();

        void OnSpawn(float x, float y, float w, float h) override;
        void OnDestroy() override;

        void ProcessOverlap();
        void CheckStateWithTease();
        void CheckStateWithoutTease();
        bool Update(float deltaTime) override;

    private:
        const std::string locationName;
        const std::string spawnPointMatch;

        Physics::ObjectId physicsTriggerId{};
        Renderer::TextureId voidTextureId{};

        bool isLocationAvailable = true;
        bool tease = false;
    };
}
