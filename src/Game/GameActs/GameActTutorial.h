#pragma once

#include "GameActBase.h"
#include <Game/GameActs/GameActBase.h>
#include <Entities/EInteractable.h>
#include <Entities/ENpc.h>
#include <Events/LocationChangedEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/EntitiesEvent.h>
#include <Events/GameShutdownEvent.h>
#include <Entities/EPlayer.h>
#include <ProgressSystem/QuestsSaveData.h>
#include <EventBus/EventBus.h>
#include <Logger/Logger.h>
#include <Map/Map.h>
#include <ProgressSystem/ProgressSystem.h>
#include <DialogSystem/DialogSystem.h>
#include <Gameplay/WorldState.h>
#include <string>
#include <format>

namespace GameActs {

class ActTutorial : public GameAct {
public:
    void Initialize() {
        if (MapManager::GetCurrentMapName() != "backroad") {
            Logger::Err("Tutorial act can only be loaded on backroad map");
            return;
        }

        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &ActTutorial::OnLocationChanged);
        onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &ActTutorial::OnInteractWithEntity);
        onEntityCreated   = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &ActTutorial::OnEntityCreated);
        onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &ActTutorial::OnEntityDestroyed);
        onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &ActTutorial::OnDialogEnded);

        // NPC должен быть на первой локации!
        World::ENpc* elderNpc = dynamic_cast<World::ENpc*>(MapManager::GetEntitiesContainer().FindEntity<World::ENpc>());
        if (elderNpc) {
            elderNpc->SetHorizontalFlip(true);
        }
    }

    void OnEntityCreated(EntityCreatedEvent& e) {
        Logger::Log(std::format("[Gameplay][Tutorial] EntityCreated: {} {}", e.GetName(), e.GetType()));
        // Здесь мы полностью сбрасываем состояние игры по приколу! Поэтому здесь и очистка сохранения должна быть!
        if (e.GetName() == "kick-my" && e.GetType() == "ass") {
            Logger::Log("[Gameplay][Tutorial] Kick my ass file created. Stopping the game.");
            EventBus::instance().EmitEvent<GameShutdownEvent>();
            WorldState::RemoveFromWorldState(World::Entity::TagName{e.GetName(), e.GetType()});
        }
        if (e.GetName() == GameplayEntities::Guard) {
            if (guardWasDeleted) {
                guardWasRecreated = true;
            }
        }
    }

    void OnEntityDestroyed(EntityDestroyedEvent& e) {
        // Тут запускаем диалог после удаления старейшины
        Logger::Log(std::format("[Gameplay][Tutorial] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
        if (e.GetName() == GameplayEntities::Guard) {
            // На случай если игрок попробует удалить старейшину пока идет диалог с ним
            // мы завершим диалог и запустим новый
            if (DialogSystemManager::IsDialogActive()) {
                DialogSystemManager::EndDialog();
            }
            ProgressSystemManager::Player().fileSystemIconVisible = true;
            DialogSystemManager::StartDialog("Guard", "dialog-after-deleted");
            guardWasDeleted = true;
        }
        if (e.GetName() == GameplayEntities::Sign) {
            // Игрок может удалить дорожный знак — он останется на карте, но это будет
            // обыграно словно он стер надпсь на нём
            Logger::Log("[Gameplay][Tutorial] Road_Sign destroyed");
            DialogSystemManager::StartDialog("Utility", "roadside-info-destroyed");
        }
    }

    void OnDialogEnded(DialogEndedEvent& e) {
        Logger::Log(std::format("[Gameplay][Intro] DialogEnded: {}", e.dialogId));
        // Здесь Guard с большой буквы, т.к он так прописан в диалоговом файле
        if (e.characterId == "Guard" && e.dialogId == "tutorial-dialog-1") {
            ProgressSystemManager::Player().fileSystemIconVisible = true;
            ProgressSystemManager::SaveData();
        }
    }

    // Последний диалог dialog-8 (про kick-my-ass)
    void OnInteractWithEntity(InteractWithEntityEvent& e) {
        if (guardWasRecreated) {
            DialogSystemManager::StartDialog("Guard", "dialog-guard-was-recreated");
            return;
        }
        Logger::Log(std::format("[Gameplay][Tutorial] InteractWithEntity: {}", e.entityId));
        if (e.entityId == GameplayEntities::Guard) {
            int& guardInteractionsCount = ProgressSystemManager::Player().tutorialDialogProgress;
            if (guardInteractionsCount < 11) {
                guardInteractionsCount++;
            }
            std::string dialogId = std::format("tutorial-dialog-{}", std::to_string(guardInteractionsCount));
            DialogSystemManager::StartDialog("Guard", dialogId);
            return;
        }

        if (e.entityId == GameplayEntities::Sign) {
            Logger::Log("[Gameplay][Tutorial] Road_Sign interacted");
            DialogSystemManager::StartDialog("Utility", "roadside-info-1");
            return;
        }
    }

    void OnLocationChanged(LocationChangedEvent& e) {
        Logger::Log(std::format("[Gameplay][Tutorial] LocationChanged: {}", e.locationName));
        if (e.locationName == "crossroads") {
            ::GameplayLogic::LoadGameAct(GameActIds::Main);
        }

        if (e.locationName == "world-void") {
            if (auto* player = MapManager::GetEntitiesContainer().FindEntity<World::EPlayer>()) {
                Renderer::AnimationHandle animation = Renderer::AnimationHandle {
                    .numOfFrames = 16,
                    .maxElementsPerRow = 4,
                    .frameSize = 64,
                    .frameDelay = 0.05f,
                    .textureId = make_nnTex("matrics"),
                };
                MapManager::SpawnEffect(player->GetPosition().x, player->GetPosition().y, 64, 64, animation);

                if (ProgressSystemManager::Quests().GetStatus(World::VoidFirstEntranceQuest) == QuestStatus::NotStarted) {
                    ProgressSystemManager::Quests().SetStatus(World::VoidFirstEntranceQuest, QuestStatus::OnGoing);
                    DialogSystemManager::StartDialog("Utility", "void-backroad-first");
                }
            }
        }
    }

private:
    Events::Handler onInteractWithEntity;
    Events::Handler onEntityCreated;
    Events::Handler onEntityDestroyed;
    Events::Handler onDialogEnded;
    Events::Handler onLocationChanged;

    bool guardWasDeleted = false;
    bool guardWasRecreated = false;
};

} // namespace GameActs