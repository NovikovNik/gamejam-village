#pragma once
#include <Renderer/Renderer.h>
#include <string>
#include "EMapObject.h"

namespace World {
    class ETriggerLocation : public EMapObject {
    public:
        ETriggerLocation(const std::string& locationName) : locationName(locationName) {}
        void Render(float deltaTime) override;
        void ChangeLocation();

    private:
        const std::string locationName;
    };
}
