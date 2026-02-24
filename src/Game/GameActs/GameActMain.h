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
#include <cstdlib>

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
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();

        /* ELDER DIALOGES */
        if (e.entityId == GameplayEntities::Elder) {
            // Первый диалог со старейшиной в деревне. Он заберет письмо и попросит пройтись через деревню
            if (currentActiveQuestId == World::ElderFirstMeetingQuest) {
                if (ProgressSystemManager::Quests().GetStatus(World::ElderFirstMeetingQuest) == QuestStatus::NotStarted) {
                    DialogSystemManager::StartDialog("Elder", "dialog-welcome");
                    if (ProgressSystemManager::Inventory().HasItem(World::message)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::message);
                    }
                    ProgressSystemManager::Quests().SetStatus(World::ElderFirstMeetingQuest, QuestStatus::OnGoing);
                }  
                if (ProgressSystemManager::Quests().GetStatus(World::ElderFirstMeetingQuest) == QuestStatus::OnGoing) {
                    // Если игрок так и остался в деревне, не выходил из неё, то мы ему говорим продолжить путь
                    if (!locationChanged) {
                        DialogSystemManager::StartDialog("Elder", "dialog-go-through-location");
                    } else {
                        ProgressSystemManager::Quests().SetStatus(World::ElderFirstMeetingQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderCatQuest);
                        DialogSystemManager::StartDialog("Elder", "dialog-cat-quest");
                    }
                }
            }

            if (currentActiveQuestId == World::ElderCatQuest) {
                const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                auto it = registeredEntities.find("cat.vil");
                const bool catNotInEldersHouse = (it == registeredEntities.end() || !it->second.contains("elders-house"));
                if (catNotInEldersHouse) {
                    DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-again");
                } else {
                    ProgressSystemManager::Quests().SetStatus(World::ElderCatQuest, QuestStatus::Completed);
                    ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderSpawnGuardQuest);
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest");
                }
            }

            if (currentActiveQuestId == World::ElderSpawnGuardQuest) {
                const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                auto it = registeredEntities.find("guard.vil");
                const bool guardNotInVillage = (it == registeredEntities.end() || !it->second.contains("crossroads"));
                if (guardNotInVillage) {
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-again");
                } else {
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
                    ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                    ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderGuardInteractQuest);
                }
            }

            if (currentActiveQuestId == World::ElderGuardInteractQuest) {
                DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
            }

            if (currentActiveQuestId == World::ElderVoidMistQuest) {
                if (ProgressSystemManager::Inventory().HasItem(World::book)) {
                    ProgressSystemManager::Inventory().RemoveItem(World::book);
                    DialogSystemManager::StartDialog("Elder", "dialog-void-mist-info");
                    ProgressSystemManager::Quests().SetStatus(World::ElderVoidMistQuest, QuestStatus::Completed);
                    ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistExtraQuest);
                } else {
                    DialogSystemManager::StartDialog("Elder", "dialog-book-quest");
                }
            }
        }

        /* GUARD DIALOGES */
        if (e.entityId == GameplayEntities::Guard) {
            if (mapName == "crossroads") {
                const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
                const std::string assemblyHallId = "assembly-hall";
                if (registeredLocations.contains(assemblyHallId) && registeredLocations.at(assemblyHallId)) {
                    DialogSystemManager::StartDialog("Guard", "dialog-assembly-hall-available");
                }

                if (currentActiveQuestId == World::ElderGuardInteractQuest) {
                    if (ProgressSystemManager::Quests().GetStatus(World::ElderGuardInteractQuest) == QuestStatus::OnGoing) {
                        if (ProgressSystemManager::Inventory().HasItem(World::sword)) {
                            ProgressSystemManager::Inventory().RemoveItem(World::sword);
                            DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed");
                            ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::Completed);
                            ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                            ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistQuest);
                        } else {
                            DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-reminder");
                        }
                    } else if (ProgressSystemManager::Quests().GetStatus(World::ElderGuardInteractQuest) == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest");
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::OnGoing);
                    }
                } else if (currentActiveQuestId == World::ElderSpawnGuardQuest || currentActiveQuestId == World::ElderFirstMeetingQuest) {
                    // Если взаимодействуем до квеста старейшины со стражником
                    if (ProgressSystemManager::Inventory().HasItem(World::sword)) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-already-found-sword");
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistQuest);
                    } else {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest");
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderGuardInteractQuest);
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::OnGoing);
                    }
                }
                if (currentActiveQuestId == World::ElderVoidMistQuest) {
                    if (std::rand() % 2 == 0) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed-reminder-1");
                    } else {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed-reminder-2");
                    }
                }
                if (currentActiveQuestId == World::ElderVoidMistExtraQuest) {
                    DialogSystemManager::StartDialog("Guard", "dialog-rebuild-assembly-hall");
                }
            }
        }

        /* JOE DIALOGES */
        if (e.entityId == GameplayEntities::Joe) {
            if (mapName == "crossroads") {
                auto joeQuestStatus = ProgressSystemManager::Quests().GetStatus(World::JoeCarrotQuest);

                if (joeQuestStatus == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Joe", "quest-start");
                        ProgressSystemManager::Quests().SetStatus(World::JoeCarrotQuest, QuestStatus::OnGoing);
                } else if (joeQuestStatus == QuestStatus::OnGoing) {
                    DialogSystemManager::StartDialog("Joe", "quest-progress");
    
                } else if (joeQuestStatus == QuestStatus::Completed && ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal) == QuestStatus::NotStarted) {
                    DialogSystemManager::StartDialog("Joe", "quest-complete");
                    ProgressSystemManager::Inventory().AddItem(World::carrot);
                    ProgressSystemManager::Quests().SetStatus(World::JoeCarrotFinal, QuestStatus::Completed);
                    ProgressSystemManager::SaveData();

                } else if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal) == QuestStatus::Completed) {
                    if (std::rand() % 2 == 0) {
                        DialogSystemManager::StartDialog("Joe", "neutral-dialog-1");
                    } else {
                        DialogSystemManager::StartDialog("Joe", "neutral-dialog-2");
                    }
                }
            }
        }

        /* COW DIALOGES */
        // Самые важные отношения с коровами в игре
        if (e.entityId == GameplayEntities::Cow) {
            if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotQuest) == QuestStatus::Completed) {
                if (mapName == "crossroads") {
                    if (ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest) == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Cow", "dialog-cow-quest-carrot");
                        if (ProgressSystemManager::Inventory().HasItem(World::carrot)) {
                            ProgressSystemManager::Inventory().RemoveItem(World::carrot);
                            ProgressSystemManager::Quests().SetStatus(World::CowFeededQuest, QuestStatus::Completed);
                            ProgressSystemManager::SaveData();
                        }
                    }
                    if (ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest) == QuestStatus::Completed) {
                        DialogSystemManager::StartDialog("Cow", "dialog-cow-quest-complete");
                    }
                }
            } else {
                DialogSystemManager::StartDialog("Cow", "dialog-cow-idle");
            }
        }

        if (e.entityId == GameplayEntities::ChestBox) {
            if (mapName == "elders-house") {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::chestBoxElderHouse)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                } else {
                    ProgressSystemManager::Inventory().SetChestBoxOpened(World::chestBoxElderHouse);
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-get-key");
                    ProgressSystemManager::Inventory().AddItem(World::key);
                }
            }
            // Здесь добавляем книгу в инвентарь (нужна старейшине)
            if (mapName == "old-house") {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::chestBoxOldHouse)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                } else {
                    if (ProgressSystemManager::Inventory().HasItem(World::key)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::key);
                        ProgressSystemManager::Inventory().AddItem(World::book);
                        ProgressSystemManager::Inventory().SetChestBoxOpened(World::chestBoxOldHouse);
                        DialogSystemManager::StartDialog("Utility", "dialog-chestbox-use-key");
                    } else {
                        DialogSystemManager::StartDialog("Utility", "dialog-chestbox-no-key");
                    }
                }
            }
        }

        if (e.entityId == GameplayEntities::Sword) {
            if (mapName == "old-house") {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::swordBox)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                } else {
                    ProgressSystemManager::Inventory().SetChestBoxOpened(World::swordBox);
                    ProgressSystemManager::Inventory().AddItem(World::sword);
                    DialogSystemManager::StartDialog("Utility", "dialog-get-sword");
                }
            }
        }
    }

    void OnEntityCreated(EntityCreatedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityCreated: {} {}", e.GetName(), e.GetType()));
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();
        std::string const currentMapName = MapManager::GetCurrentMapName();
        if (currentMapName == "elders-house") {
            if (e.GetName() == "cat") {
                // Кот в доме старейшины если старейшина есть внутри
                if (currentActiveQuestId == World::ElderCatQuest) {
                    auto* elderNpc = MapManager::GetEntitiesContainer().FindEntity({"Elder", "vil"});
                    if (elderNpc) {
                        ProgressSystemManager::Quests().SetStatus(World::ElderCatQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderSpawnGuardQuest);
                        DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest");
                    }
                }
            }
        }
    }

    void OnEntityDestroyed(EntityDestroyedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
        if (e.GetName() == "box-1" || e.GetName() == "box-2") {
            if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotQuest) == QuestStatus::OnGoing) {
                ProgressSystemManager::Quests().SetStatus(World::JoeCarrotQuest, QuestStatus::Completed);
            }
            if (ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest) == QuestStatus::NotStarted) {
                ProgressSystemManager::Quests().SetStatus(World::CowFeededQuest, QuestStatus::OnGoing);
            }
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