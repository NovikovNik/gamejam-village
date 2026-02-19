#include "ENpc.h"
#include <Logger/Logger.h>
#include <format>
#include <Events/ForceDialogStartEvent.h>
#include <Events/InteractWithEntityEvent.h>

World::ENpc::ENpc(const std::string& name, float x, float y) : EInteractable(make_nnInteractId(name)) {
    const auto textureId = make_nnTex(std::format("npc_{}", name));
    LoadData(textureId, x, y, 64, 64);
}


bool World::ENpc::Update(float deltaTime) {
    return EInteractable::Update(deltaTime);
}

void World::ENpc::Render(float deltaTime) {
    EInteractable::Render(deltaTime);
}

void World::ENpc::Interact() {
    Logger::Log(std::format("[ENpc] Emit interact with entity event: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}
