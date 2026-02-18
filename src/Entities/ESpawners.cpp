#include "ESpawners.h"
#include <Renderer/Renderer.h>
#include <Entities/EBox.h>
#include <Entities/ENpc.h>
#include <Map/Map.h>

bool World::ESpawners::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::ESpawners::Render(float deltaTime) {
    const auto textureId = Renderer::TextureId("spawner"_nnTex);
    for (const auto& spawner : spawners) {
        Renderer::DrawSprite(textureId, spawner.x, spawner.y, 32, 32);
    }
}

void World::ESpawners::LoadSpawners(const std::vector<Spawner>& spawners) {
    this->spawners = spawners;
}

std::optional<glm::vec2> World::ESpawners::GetSpawnerPosition(const std::string& name, const std::string& type) const {
    for (const auto& spawner : spawners) {
        if (spawner.name == name && spawner.type == type) {
            return glm::vec2(spawner.x, spawner.y);
        }
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, std::string>> World::ESpawners::GetInstantSpawnEntities() const {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& spawner : spawners) {
        if (spawner.shouldSpawnInstantly) {
            result.emplace_back(spawner.name, spawner.type);
        }
    }
    return result;
}
//World::Entity* World::ESpawners::SpawnEntity(const std::string& name, const std::string& type, const std::function<Entity*()>& spawnFunction) {
//
//}
