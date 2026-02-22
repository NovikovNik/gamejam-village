#include "ENpc.h"
#include <Logger/Logger.h>
#include <format>
#include <Events/ForceDialogStartEvent.h>
#include <Events/InteractWithEntityEvent.h>

World::ENpc::ENpc(const std::string& name) : EInteractable(make_nnInteractId(name)) {
    const auto textureId = make_nnTex(std::format("npc_{}", name));
    if (name != "Cow") {
        LoadData(textureId, 64, 64);

        animationIdle.textureId = make_nnTex(std::format("npc_{}_idle", name));
        animationIdle.numOfFrames = 6;
        animationIdle.maxElementsPerRow = 6;
        animationIdle.frameSize = 64;
        animationIdle.frameDelay = 0.15f;
    } else {
        LoadData(textureId, 128, 64);

        animationIdle.textureId = make_nnTex(std::format("npc_{}_idle", name));
        animationIdle.numOfFrames = 6;
        animationIdle.maxElementsPerRow = 6;
        animationIdle.frameSize = 128;
        animationIdle.frameDelay = 0.15f;
    }
}

bool World::ENpc::Update(float deltaTime) {
    return EInteractable::Update(deltaTime);
}

void World::ENpc::Render(float deltaTime) {
    if (Renderer::HasTexture(animationIdle.textureId)) {
        Renderer::RenderAnimation(animationIdle, deltaTime, positionX, positionY, width, height, horizontalFlip);
    } else {
        Renderer::DrawSprite(texture, positionX, positionY, width, height, 0.0, horizontalFlip);
    }
}

void World::ENpc::Interact() {
    Logger::Log(std::format("[ENpc] Emit interact with entity event: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}

void World::ENpc::OnSpawn(float x, float y, float w, float h) {
    EInteractable::OnSpawn(x, y, w, h);
}

void World::ENpc::CreatePhysicsObjects() {
    physicsColliderId = Physics::CreateStaticRectangle(GetPosition().x, GetPosition().y, GetWidth() * 0.5f, GetHeight() * 0.9f);
    physicsTriggerId = Physics::CreateStaticRectangle(GetPosition().x, GetPosition().y, GetWidth(), GetHeight(), true);
}

void World::ENpc::RemovePhysicsObjects() {
    Physics::RemoveObject(physicsColliderId);
    Physics::RemoveObject(physicsTriggerId);
}
