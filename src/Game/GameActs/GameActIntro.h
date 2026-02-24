#pragma once

#include "GameActBase.h"
#include <Game/GameActs/GameActBase.h>
#include <Entities/EInteractable.h>
#include <Events/LocationChangedEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <EventBus/EventBus.h>
#include <Logger/Logger.h>
#include <Map/Map.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Gameplay/WorldState.h>
#include <string>
#include <format>

namespace GameActs {

class ActIntro : public GameAct {
public:
    void Initialize() {
        if (MapManager::GetCurrentMapName() != "intro") {
            Logger::Err("Intro act can only be loaded on intro map");
            return;
        }

        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &ActIntro::OnLocationChanged);
        onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &ActIntro::OnDialogEnded);
        EventBus::instance().EmitEvent<ForceDialogStartEvent>("Intro", "intro-1");

        penchamentEntity = MapManager::GetEntitiesContainer().FindEntity<World::EInteractable>();
        if (penchamentEntity) {
            baseBoxY = penchamentEntity->GetPosition().y;
            timeAccumulator = 0.0f;
        }

        // Здесь мы добавляем письмо (сюжетный обьект) в инвентарь!!!
        ProgressSystemManager::Inventory().AddItem(World::message);
    }

    void Update(float deltaTime) override {
        // В этом апдейте делаем просто красивое покачивание 
        // письма по Y синусоиде
        if (!penchamentEntity) {
            return;
        }

        timeAccumulator += deltaTime;

        // само движение
        const float amplitude = 5.0f;
        const float frequency = 1.0f;
        const float offsetY = std::sin(timeAccumulator * frequency) * amplitude;

        const auto currentPos = penchamentEntity->GetPosition();
        penchamentEntity->SetPosition(currentPos.x, baseBoxY + offsetY);
    }

    void OnDialogEnded(DialogEndedEvent& e) {
        Logger::Log(std::format("[Gameplay][Intro] DialogEnded: {}", e.dialogId));
        if (e.characterId == "Intro" && e.dialogId == "intro-1") {
            EventBus::instance().EmitEvent<ChangeLocationEvent>("assets/maps/backroad.json");
        }
    }

    void OnLocationChanged(LocationChangedEvent& e) {
        penchamentEntity = nullptr;
        if (e.locationName == "backroad") {
            ::GameplayLogic::LoadGameAct(GameActIds::Tutorial);
        }
    }

private:
    Events::Handler onDialogEnded;
    Events::Handler onLocationChanged;

    World::EInteractable* penchamentEntity = nullptr;
    float baseBoxY = 0.0f;
    float timeAccumulator = 0.0f;
};

} // namespace GameActs