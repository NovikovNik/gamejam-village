#include "EPit.h"
#include <Events/PitEvent.h>

void World::EPit::OnSpawn(float x, float y, float w, float h) {
    EMovable::OnSpawn(x, y, w, h);
    const auto textureId = Renderer::TextureId("pit"_nnTex);
    LoadData(textureId, 64, 64);
    physicsPlayerColliderId = Physics::CreateStaticRectangle(x, y, GetWidth(), GetHeight(), false, 1 << 2);
    physicsTriggerId = Physics::CreateStaticRectangle(x, y, GetWidth() * 0.5f, GetHeight() * 0.5f, true, 1 << 4);
}

bool World::EPit::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }


    const auto& overlapInfos = Physics::GetOverlapInfos();
    for (const auto& overlapInfo : overlapInfos) {
        if (overlapInfo.objectId1 == physicsTriggerId || overlapInfo.objectId2 == physicsTriggerId) {
            const auto boxPhysicsId = overlapInfo.objectId2 == physicsTriggerId ? overlapInfo.objectId1 : overlapInfo.objectId2;
            EventBus::instance().EmitEvent<PitBoxOverlapEvent>(matchBoxName, GetTagName(), boxPhysicsId);
            break;
        }
    }
    return isValid;
}

void World::EPit::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}

bool World::EPit::IsBoxNameMatch(BoxName boxName) const {
    return matchBoxName == boxName;
}

void World::EPit::OnDestroy() {
    EMovable::OnDestroy();
    Physics::RemoveObject(physicsPlayerColliderId);
    Physics::RemoveObject(physicsTriggerId);
}
