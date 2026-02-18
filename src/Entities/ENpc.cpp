#include "ENpc.h"
#include <Logger/Logger.h>
#include <format>
#include <Events/ForceDialogStartEvent.h>
#include <EventBus/EventsQueue.h>

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
    Logger::Log(std::format("Interact with NPC: {}", get_nnInteractId(GetInteractId())));
    EventsQueue::instance().Push(ForceDialogStartEvent(get_nnInteractId(GetInteractId()), "dialog-1"));
}
