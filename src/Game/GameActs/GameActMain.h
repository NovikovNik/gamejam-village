#pragma once

#include "GameActBase.h"
#include <Entities/EInteractable.h>
#include <Events/LocationChangedEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/EntitiesEvent.h>
#include <EventBus/EventBus.h>
#include <Logger/Logger.h>
#include <Map/Map.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Gameplay/WorldState.h>
#include <DialogSystem/DialogSystem.h>
#include <string>
#include <format>

namespace GameActs {

class MainAct : public GameAct {
public:
    void Initialize() {
        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);
        onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &MainAct::OnInteractWithEntity);
        onEntityCreated = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &MainAct::OnEntityCreated);
        onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &MainAct::OnEntityDestroyed);
        onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &MainAct::OnDialogEnded);
        onLocationChangedEvent = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);
    }

    void Update(float /*deltaTime*/) override {}

    void OnLocationChanged(LocationChangedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] LocationChanged: {}", e.locationName));
        if (e.locationName == "backroad") {
            locationChanged = true;
        }
    }

    void OnInteractWithEntity(InteractWithEntityEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] InteractWithEntity: {}", e.entityId));
        std::string const mapName = MapManager::GetCurrentMapName();

        if (e.entityId == GameplayEntities::Elder) {
            // Первый диалог со старейшиной в деревне. Он заберет письмо и попросит пройтись через деревню
            if (ProgressSystemManager::Player().elderHubActiveQuest == "go-through-location") {
                if (!elderAskToGoThroughLocation) {
                    DialogSystemManager::StartDialog("Elder", "dialog-welcome");
                    if (ProgressSystemManager::Inventory().HasItem(World::message)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::message);
                    }
                    elderAskToGoThroughLocation = true;
                } else {
                    // Если игрок так и остался в деревне, не выходил из неё, то мы ему говорим продолжить путь
                    if (!locationChanged) {
                        DialogSystemManager::StartDialog("Elder", "dialog-go-through-location");
                    } else {
                        ProgressSystemManager::Player().elderHubActiveQuest = "cat-quest";
                        DialogSystemManager::StartDialog("Elder", "dialog-cat-quest");
                    }
                }
            }
            if (ProgressSystemManager::Player().elderHubActiveQuest == "cat-quest") {
                const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                auto it = registeredEntities.find("cat.vil");
                const bool catNotInEldersHouse = (it == registeredEntities.end() || !it->second.contains("elders-house"));
                if (catNotInEldersHouse) {
                    DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-again");
                } else {
                    ProgressSystemManager::Player().elderHubActiveQuest = "spawn-guard";
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest");
                }
            }
            if (ProgressSystemManager::Player().elderHubActiveQuest == "spawn-guard") {
                const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                auto it = registeredEntities.find("guard.vil");
                const bool guardNotInVillage = (it == registeredEntities.end() || !it->second.contains("crossroads"));
                if (guardNotInVillage) {
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-again");
                } else {
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
                    ProgressSystemManager::Player().elderHubActiveQuest = "guard_quest";
                }
            }
            if (ProgressSystemManager::Player().elderHubActiveQuest == "guard_quest") {
                DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
            }
            if (ProgressSystemManager::Player().elderHubActiveQuest == "void-mist") {
                if (ProgressSystemManager::Inventory().HasItem(World::book)) {
                    ProgressSystemManager::Inventory().RemoveItem(World::book);
                    DialogSystemManager::StartDialog("Elder", "dialog-void-mist-info");
                    ProgressSystemManager::Player().elderHubActiveQuest = "void-mist-info";
                } else {
                    DialogSystemManager::StartDialog("Elder", "dialog-book-quest");
                }
            }
        }

        if (e.entityId == GameplayEntities::Guard) {
            if (mapName == "crossroads") {
                const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
                const std::string assemblyHallId = "assembly-hall";
                if (registeredLocations.contains(assemblyHallId) && registeredLocations.at(assemblyHallId)) {
                    DialogSystemManager::StartDialog("Guard", "dialog-assembly-hall-available");
                }

                if (ProgressSystemManager::Player().elderHubActiveQuest == "guard_quest" || ProgressSystemManager::Player().elderHubActiveQuest == "spawn-guard") {
                    if (ProgressSystemManager::Inventory().HasItem(World::sword)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::sword);
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed");
                        ProgressSystemManager::Player().elderHubActiveQuest = "void-mist";
                    } else {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest");
                    }
                }
                if (ProgressSystemManager::Player().elderHubActiveQuest == "void-mist") {
                    DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed");
                }
                if (ProgressSystemManager::Player().elderHubActiveQuest == "void-mist-info") {
                    DialogSystemManager::StartDialog("Guard", "dialog-rebuild-assembly-hall");
                }
            }
        }

        if (e.entityId == GameplayEntities::Joe) {
            if (ProgressSystemManager::Player().joeQuestProgress == 0) {
                DialogSystemManager::StartDialog("Joe", "quest-start");

            } else if (ProgressSystemManager::Player().joeQuestProgress == 1) {
                DialogSystemManager::StartDialog("Joe", "quest-progress");

            } else if (ProgressSystemManager::Player().joeQuestProgress == 2) {
                DialogSystemManager::StartDialog("Joe", "quest-complete");
                ProgressSystemManager::Inventory().AddItem(World::carrot);
                ProgressSystemManager::SaveData();
            }
        }

        // Самые важные отношения с коровами в игре
        if (e.entityId == GameplayEntities::Cow) {
            if (ProgressSystemManager::Player().joeQuestProgress >= 2) {
                if (mapName == "crossroads") {
                    DialogSystemManager::StartDialog("Cow", "dialog-cow-quest-carrot");
                    if (ProgressSystemManager::Inventory().HasItem(World::carrot)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::carrot);
                        ProgressSystemManager::Player().cowQuestFeeded = true;
                        ProgressSystemManager::SaveData();
                    }
                }
            } else {
                DialogSystemManager::StartDialog("Cow", "dialog-cow-idle");
            }
        }

        if (e.entityId == GameplayEntities::ChestBox) {
            if (mapName == "elders-house") {
                if (!ProgressSystemManager::Inventory().HasItem(World::key)) {
                    ProgressSystemManager::Inventory().AddItem(World::key);
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-get-key");
                }
            }
            // Здесь добавляем книгу в инвентарь (нужна старейшине)
            if (mapName == "old-house") {
                if (ProgressSystemManager::Inventory().HasItem(World::key)) {
                    ProgressSystemManager::Inventory().RemoveItem(World::key);
                    ProgressSystemManager::Inventory().AddItem(World::book);
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-use-key");
                } else {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-no-key");
                }
            }
        }
        if (e.entityId == "Sword") {
            if (mapName == "old-house") {
                if (!ProgressSystemManager::Inventory().HasItem(World::sword)) {
                    ProgressSystemManager::Inventory().AddItem(World::sword);
                    DialogSystemManager::StartDialog("Utility", "dialog-get-sword");
                }
            }
        }
    }

    void OnEntityCreated(EntityCreatedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityCreated: {} {}", e.GetName(), e.GetType()));
        if (MapManager::GetCurrentMapName() == "elders-house") {
            if (e.GetName() == "cat") {
                // Кот в доме старейшины если старейшина есть внутри
                if (ProgressSystemManager::Player().elderHubActiveQuest == "cat-quest") {
                    auto* elderNpc = MapManager::GetEntitiesContainer().FindEntity({"Elder", "vil"});
                    if (elderNpc) {
                        ProgressSystemManager::Player().elderHubActiveQuest = "spawn-guard";
                        DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest");
                    }
                }
            }
        }
    }

    void OnEntityDestroyed(EntityDestroyedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
        if (e.GetName() == "box-1" || e.GetName() == "box-2") {
            ProgressSystemManager::Player().joeQuestProgress++;
        }
    }

    void OnDialogEnded(DialogEndedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] DialogEnded: {}", e.dialogId));
    }

private:
    int elderInteractionsCount = 0;
    bool locationChanged = false;
    bool elderAskToGoThroughLocation = false;

    Events::Handler onLocationChangedEvent;
    Events::Handler onLocationChanged;
    Events::Handler onInteractWithEntity;
    Events::Handler onEntityCreated;
    Events::Handler onEntityDestroyed;
    Events::Handler onDialogEnded;
    Events::Handler onFocusWindow;
};

} // namespace GameActs