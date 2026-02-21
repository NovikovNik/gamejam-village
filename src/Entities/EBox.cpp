#include "EBox.h"
#include <Map/Map.h>
//#include <Entities/EColliders.h>
#include <Physics/PhysicsEngine.h>
#include <Entities/EPit.h>
#include <Gameplay/WorldState.h>

void World::EBox::OnSpawn(float x, float y, float w, float h) {
    EMovable::OnSpawn(x, y, w, h);
    const auto textureId = Renderer::TextureId("box"_nnTex);
    LoadData(textureId, 64, 64);
    physicsObjectId = Physics::CreateDynamicRectangle(x, y, GetWidth(), GetHeight());
}

void World::EBox::OnDestroy() {
    Physics::RemoveObject(physicsObjectId);
}

bool World::EBox::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }
    
    const auto& movableObjectPositions = Physics::GetMovableObjectPositions();
    for (const auto& movableObjectPosition : movableObjectPositions) {
        if (movableObjectPosition.id == physicsObjectId) {
            SetPosition(movableObjectPosition.x, movableObjectPosition.y);
            break;
        }
    }

    const auto& overlapInfos = Physics::GetOverlapInfos();
    for (const auto& overlapInfo : overlapInfos) {
        if (overlapInfo.objectId1 == physicsObjectId || overlapInfo.objectId2 == physicsObjectId) {
            WorldState::RegisterInWorldState(GetTagName());
            break;
        }
    }
    return true;
}

void World::EBox::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}

bool World::EBox::CanMove(float dirX, float dirY, float speed, float deltaTime) const {
//    glm::vec2 pos = GetPosition();
//    float moveX = dirX * speed * deltaTime;
//    float moveY = dirY * speed * deltaTime;
//
////    std::vector<World::EColliders*> colliders;
////    MapManager::GetEntitiesContainer().FindEntities(colliders);
//
//    auto WouldCollide = [&](float dx, float dy) {
//        float pl = pos.x + dx - GetWidth() * 0.5f;
//        float pr = pos.x + dx + GetWidth() * 0.5f;
//        float pt = pos.y + dy - GetHeight() * 0.5f;
//        float pb = pos.y + dy + GetHeight() * 0.5f;
//
//        for (auto* ec : colliders) {
//            for (const auto& c : ec->GetColliders()) {
//                float cl = c.x - c.width * 0.5f;
//                float cr = c.x + c.width * 0.5f;
//                float ct = c.y - c.height * 0.5f;
//                float cb = c.y + c.height * 0.5f;
//                if (pl < cr && pr > cl && pt < cb && pb > ct) {
//                    return true;
//                }
//            }
//        }
//        return false;
//    };
//
//    if (dirX != 0.0f && WouldCollide(moveX, 0)) return false;
//    if (dirY != 0.0f && WouldCollide(0, moveY)) return false;
    return true;
}

void World::EBox::Move(float dirX, float dirY, float speed, float deltaTime) {
//    glm::vec2 pos = GetPosition();
//    float moveX = dirX * speed * deltaTime;
//    float moveY = dirY * speed * deltaTime;
//
//    std::vector<World::EColliders*> colliders;
//    MapManager::GetEntitiesContainer().FindEntities(colliders);
//
//    auto WouldCollide = [&](float dx, float dy) {
//        float pl = pos.x + dx - GetWidth() * 0.5f;
//        float pr = pos.x + dx + GetWidth() * 0.5f;
//        float pt = pos.y + dy - GetHeight() * 0.5f;
//        float pb = pos.y + dy + GetHeight() * 0.5f;
//
//        for (auto* ec : colliders) {
//            for (const auto& c : ec->GetColliders()) {
//                float cl = c.x - c.width * 0.5f;
//                float cr = c.x + c.width * 0.5f;
//                float ct = c.y - c.height * 0.5f;
//                float cb = c.y + c.height * 0.5f;
//                if (pl < cr && pr > cl && pt < cb && pb > ct) {
//                    return true;
//                }
//            }
//        }
//        return false;
//    };
//
//    float impulseX = (WouldCollide(moveX, 0) ? 0.0f : dirX) * speed;
//    float impulseY = (WouldCollide(0, moveY) ? 0.0f : dirY) * speed;
//
//    AddImpulse(impulseX, impulseY);
//
//    std::vector<World::EPit*> pits;
//    MapManager::GetEntitiesContainer().FindEntities(pits);
//    for (const auto& pit : pits) {
//        if (!pit->IsValid()) continue;
//        if (pit->IsBoxNameMatch(boxName)) {
//            const auto pitPosition = pit->GetPosition();
//            const auto pitWidth = pit->GetWidth();
//            const auto boxPosition = GetPosition();
//            const auto distance = glm::distance(pitPosition, boxPosition);
//            if (distance < pitWidth * 0.7f) { // 0.7f это коэффициент вывереный на глаз
//                MapManager::DestroyEntity(pit->GetTagName().name, pit->GetTagName().type);
//                MapManager::DestroyEntity(GetTagName().name, GetTagName().type);
//            }
//            return;
//        }
//    }
}
