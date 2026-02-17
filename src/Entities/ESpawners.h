#pragma once

#include "Entity.h"
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <optional>

namespace World {
    class ESpawners : public Entity {
    public:
        struct Spawner {
            std::string name;
            std::string type;
            float x;
            float y;
        };
    public:
        bool Update(float deltaTime) override;
        void Render(float deltaTime) override;

        void LoadSpawners(const std::vector<Spawner>& spawners);

        [[nodiscard]] std::optional<glm::vec2> GetSpawnerPosition(const std::string& name, const std::string& type) const;

    private:
        std::vector<Spawner> spawners;
    };
}
