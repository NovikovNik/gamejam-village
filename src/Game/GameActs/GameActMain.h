#pragma once

#include "GameActBase.h"
#include <Entities/EInteractable.h>
#include <Entities/EEffect.h>
#include <Events/LocationChangedEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/EntitiesEvent.h>
#include <Events/PlaySoundEvent.h>
#include <EventBus/EventBus.h>
#include <Entities/EntitiesManager.h>
#include <Entities/EPlayer.h>
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
    void ProcessEnteringVoid()
    {
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();
        if (currentActiveQuestId != World::FinalActQuest) {
            /* VOID FIRST ENTRANCE DIALOG */
            if (ProgressSystemManager::Quests().GetStatus(World::VoidFirstEntranceQuest) == QuestStatus::NotStarted) {
                ProgressSystemManager::Quests().SetStatus(World::VoidFirstEntranceQuest, QuestStatus::OnGoing);
                DialogSystemManager::StartDialog("Utility", "void-crossroads-first");
            }
        }
        else {
            /* В КОНЦЕ ИГРЫ ДВИГАЕМ НЕБУЛУ В НУЖНУЮ ПОЗИЦИЮ */
            World::ENpc* nebula = dynamic_cast<World::ENpc*>(MapManager::GetEntitiesContainer().FindEntity({ "nebula", "vil" }));
            if (nebula) {
                nebula->SetPosition(239, -23);
                nebula->SetHorizontalFlip(true);
                DialogSystemManager::StartDialog("Nebula", "entrance-dialog");
            }
        }
    }
    void Initialize() override {
        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);
        onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &MainAct::OnInteractWithEntity);
        onEntityCreated = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &MainAct::OnEntityCreated);
        onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &MainAct::OnEntityDestroyed);
        onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &MainAct::OnDialogEnded);
        onLocationChangedEvent = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);

        if (MapManager::GetCurrentMapName() == GameplayMaps::WorldVoid)
        {
            ProcessEnteringVoid();
        }

        if (MapManager::GetCurrentMapName() == GameplayMaps::AssemblyHall) {
            ProgressSystemManager::Quests().SetCurrentActiveQuest(World::FinalActQuest); // Делаем последний квест активным при переходе в ассембли
        }
    }

    void Update(float /*deltaTime*/) override {}

    void OnLocationChanged(LocationChangedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] LocationChanged: {}", e.locationName));
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();

        if (e.locationName == GameplayMaps::Backroad) {
            locationChanged = true;
        }
        // Spawn effect when player enters world-void
        if (e.locationName == GameplayMaps::WorldVoid) {
            if (auto* player = MapManager::GetEntitiesContainer().FindEntity<World::EPlayer>()) {
                Renderer::AnimationHandle animation = Renderer::AnimationHandle{
                    .numOfFrames = 16,
                    .maxElementsPerRow = 4,
                    .frameSize = 64,
                    .frameDelay = 0.05f,
                    .textureId = make_nnTex("matrics"),
                };
                MapManager::SpawnEffect(player->GetPosition().x, player->GetPosition().y, 64, 64, animation);
                ProcessEnteringVoid();
            }
        }

        if (MapManager::GetCurrentMapName() == GameplayMaps::AssemblyHall) {
            ProgressSystemManager::Quests().SetCurrentActiveQuest(World::FinalActQuest); // Делаем последний квест активным при переходе в ассембли
        }

        /* CAT LOCATION during Cat Quest! */
        if (e.locationName == GameplayMaps::EldersHouse) {
            if (currentActiveQuestId == World::ElderCatQuest) {
                auto* cat = MapManager::GetEntitiesContainer().FindEntity({"cat", "vil"});
                if (cat && ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::OnGoing) {
                    cat->SetPosition(-143, 88);
                }
                if (cat && ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::Completed) {
                    cat->SetPosition(149, 88);
                }
            }
        }
    }

    void OnInteractWithEntity(InteractWithEntityEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] InteractWithEntity: {}", e.entityId));
        std::string const mapName = MapManager::GetCurrentMapName();
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();

        /* ELDER DIALOGES */
        if (e.entityId == GameplayEntities::Elder) {
            if (mapName != GameplayMaps::AssemblyHall) {
                // Первый диалог со старейшиной в деревне. Он заберет письмо и попросит пройтись через деревню
                if (currentActiveQuestId == World::ElderFirstMeetingQuest) {
                    if (ProgressSystemManager::Quests().GetStatus(World::ElderFirstMeetingQuest) == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Elder", "dialog-welcome");
                        if (ProgressSystemManager::Inventory().HasItem(World::message)) {
                            ProgressSystemManager::Inventory().RemoveItem(World::message);
                        }
                        ProgressSystemManager::Quests().SetStatus(World::ElderFirstMeetingQuest, QuestStatus::OnGoing);
                        locationChanged = false;
                        return;
                    }

                    if (ProgressSystemManager::Quests().GetStatus(World::ElderFirstMeetingQuest) == QuestStatus::OnGoing) {
                        // Если игрок так и остался в деревне, не выходил из неё, то мы ему говорим продолжить путь
                        if (!locationChanged) {
                            DialogSystemManager::StartDialog("Elder", "dialog-go-through-location");
                            return;
                        } else {
                            ProgressSystemManager::Quests().SetStatus(World::ElderFirstMeetingQuest, QuestStatus::Completed);
                            ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderCatQuest);

                            if (mapName == GameplayMaps::EldersHouse) {
                                ProgressSystemManager::Quests().SetStatus(World::ElderCatQuestInfo, QuestStatus::Completed);
                                DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-house");
                                return;
                            }
                            if (mapName == GameplayMaps::Crossroads) {
                                ProgressSystemManager::Quests().SetStatus(World::ElderCatQuestInfo, QuestStatus::Completed);
                                DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-street");
                                return;
                            }
                        }
                    }
                }

                if (currentActiveQuestId == World::ElderCatQuest) {
                    if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::NotStarted) {
                        if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuestInfo) == QuestStatus::Completed) {
                            DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-again");
                            return;
                        }
                    }

                    if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::OnGoing) {
                        DialogSystemManager::StartDialog("Elder", "dialog-cat-quest-progress");
                        return;
                    }

                    if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::Completed) {
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderSpawnGuardQuest);
                        DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest");
                        return;
                    }
                }

                if (currentActiveQuestId == World::ElderSpawnGuardQuest) {
                    const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                    auto it = registeredEntities.find("guard.vil");
                    const bool guardNotInVillage = (it == registeredEntities.end() || !it->second.contains("crossroads"));
                    if (guardNotInVillage) {
                        DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-again");
                        return;
                    } else {
                        DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
                        ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderGuardInteractQuest);
                        return;
                    }
                }

                if (currentActiveQuestId == World::ElderGuardInteractQuest) {
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-completed");
                    return;
                }

                if (currentActiveQuestId == World::ElderVoidMistQuest) {
                    if (ProgressSystemManager::Inventory().HasItem(World::book)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::book);
                        DialogSystemManager::StartDialog("Elder", "dialog-void-mist-info");
                        ProgressSystemManager::Quests().SetStatus(World::ElderVoidMistQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistExtraQuest);
                        return;
                    } else {
                        DialogSystemManager::StartDialog("Elder", "dialog-book-quest");
                        return;
                    }
                }

                if (currentActiveQuestId == World::ElderVoidMistExtraQuest) {
                    DialogSystemManager::StartDialog("Elder", "dialog-assembly-hall-rebuild");
                    return;
                }
            }
            if (mapName == GameplayMaps::AssemblyHall) {
                const int& elderAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::ElderAssembly, 3);
                if (elderAssemblyHallQuestProgress == 1) {
                    DialogSystemManager::StartDialog("Elder", "dialog-assembly-hall-quest-1");
                    return;
                }
                if (elderAssemblyHallQuestProgress == 2) {
                    if (ProgressSystemManager::Player().catWasDestroyed) {
                        DialogSystemManager::StartDialog("Elder", "dialog-assembly-hall-quest-2-cat-does-not-exist");
                    } else {
                        DialogSystemManager::StartDialog("Elder", "dialog-assembly-hall-quest-2-cat-exists");
                    }
                    return;
                }
                if (elderAssemblyHallQuestProgress >= 3) {
                    DialogSystemManager::StartDialog("Elder", "dialog-assembly-hall-quest-3");
                    return;
                }
            }
        }

        /* GUARD DIALOGES */
        if (e.entityId == GameplayEntities::Guard) {
            if (mapName == GameplayMaps::Crossroads) {
                const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
                const std::string assemblyHallId = "assembly-hall";
                if (registeredLocations.contains(assemblyHallId) && registeredLocations.at(assemblyHallId)) {
                    DialogSystemManager::StartDialog("Guard", "dialog-assembly-hall-available");
                    return;
                }

                if (currentActiveQuestId == World::ElderGuardInteractQuest) {
                    if (ProgressSystemManager::Quests().GetStatus(World::ElderGuardInteractQuest) == QuestStatus::OnGoing) {
                        if (ProgressSystemManager::Inventory().HasItem(World::sword)) {
                            ProgressSystemManager::Inventory().RemoveItem(World::sword);
                            DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed");
                            ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::Completed);
                            ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                            ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistQuest);
                            return;
                        } else {
                            DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-reminder");
                            return;
                        }
                    } else if (ProgressSystemManager::Quests().GetStatus(World::ElderGuardInteractQuest) == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest");
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::OnGoing);
                        return;
                    }
                } else if (currentActiveQuestId == World::ElderSpawnGuardQuest || currentActiveQuestId == World::ElderFirstMeetingQuest) {
                    // Если взаимодействуем до квеста старейшины со стражником
                    if (ProgressSystemManager::Inventory().HasItem(World::sword)) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-already-found-sword");
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetStatus(World::ElderSpawnGuardQuest, QuestStatus::Completed);
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderVoidMistQuest);
                        return;
                    } else {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest");
                        ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderGuardInteractQuest);
                        ProgressSystemManager::Quests().SetStatus(World::ElderGuardInteractQuest, QuestStatus::OnGoing);
                        return;
                    }
                }
                if (currentActiveQuestId == World::ElderVoidMistQuest) {
                    if (std::rand() % 2 == 0) {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed-reminder-1");
                        return;
                    } else {
                        DialogSystemManager::StartDialog("Guard", "dialog-guard-quest-completed-reminder-2");
                        return;
                    }
                }
                if (currentActiveQuestId == World::ElderVoidMistExtraQuest) {
                    DialogSystemManager::StartDialog("Guard", "dialog-rebuild-assembly-hall");
                    return;
                }
            }
            if (mapName == GameplayMaps::Backroad) {
                // Стражник скажет, что он не на своем месте
                DialogSystemManager::StartDialog("Guard", "dialog-backroad-main-act");
                return;
            }

            if (mapName == GameplayMaps::AssemblyHall) {
                const int& guardAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::GuardAssembly, 2);
                DialogSystemManager::StartDialog("Guard", std::format("dialog-assembly-hall-{}", guardAssemblyHallQuestProgress));
                return;
            }
        }

        /* JOE DIALOGES */
        if (e.entityId == GameplayEntities::Joe) {
            if (mapName == GameplayMaps::Crossroads) {
                auto joeQuestStatus = ProgressSystemManager::Quests().GetStatus(World::JoeCarrotQuest); // Boxes
                auto joeCarrotFinalQuestStatus = ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal); // Joe

                if (joeQuestStatus == QuestStatus::NotStarted) {
                    if (joeCarrotFinalQuestStatus == QuestStatus::NotStarted) {
                        ProgressSystemManager::Quests().SetStatus(World::JoeCarrotFinal, QuestStatus::OnGoing);
                        DialogSystemManager::StartDialog("Joe", "quest-start");
                        return;
                    }
                    if (joeCarrotFinalQuestStatus == QuestStatus::OnGoing) {
                        DialogSystemManager::StartDialog("Joe", "quest-repeat-task");
                        return;
                    }

                } else if (joeQuestStatus == QuestStatus::OnGoing) {

                    if (joeCarrotFinalQuestStatus == QuestStatus::NotStarted) {
                        ProgressSystemManager::Quests().SetStatus(World::JoeCarrotFinal, QuestStatus::OnGoing);
                        DialogSystemManager::StartDialog("Joe", "quest-start-without-asking");
                        return;
                    }

                    if (std::rand() % 2 == 0) {
                        DialogSystemManager::StartDialog("Joe", "quest-progress-1");
                    } else {
                        DialogSystemManager::StartDialog("Joe", "quest-progress-2");
                    }
                    return;
    
                } else if (joeQuestStatus == QuestStatus::Completed) {
                    /* Завершили квест даже не начав его!*/
                    if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal) == QuestStatus::NotStarted) {
                        DialogSystemManager::StartDialog("Joe", "quest-complete-before-start");
                        ProgressSystemManager::Inventory().AddItem(World::carrot);
                        ProgressSystemManager::Quests().SetStatus(World::JoeCarrotFinal, QuestStatus::Completed);
                        ProgressSystemManager::SaveData();
                        return;
                    }
                    /* Завершили квест нормально */
                    if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal) == QuestStatus::OnGoing) {
                        DialogSystemManager::StartDialog("Joe", "quest-complete");
                        ProgressSystemManager::Inventory().AddItem(World::carrot);
                        ProgressSystemManager::Quests().SetStatus(World::JoeCarrotFinal, QuestStatus::Completed);
                        ProgressSystemManager::SaveData();
                        return;
                    }
                } 
                if (ProgressSystemManager::Quests().GetStatus(World::JoeCarrotFinal) == QuestStatus::Completed) {
                    const int& joeCarrotQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::JoeCarrot, 5);
                    DialogSystemManager::StartDialog("Joe", std::format("neutral-dialog-{}", joeCarrotQuestProgress));
                    return;
                }
            }
            /* JOE BACKROAD SIDE QUEST */
            // Здесь идея в том, что Джо будет искать корову, которую тоже можно заспавнить на Backroad
            if (mapName == GameplayMaps::Backroad) {
                bool isCowSpawned = MapManager::GetEntitiesContainer().FindEntity({"cow", "vil"}) != nullptr;

                if (ProgressSystemManager::Quests().GetStatus(World::JoeBackroadSideQuest) == QuestStatus::NotStarted) {
                    if (!isCowSpawned) {
                        // Джо сообщит, что ищет корову
                        DialogSystemManager::StartDialog("Joe", "dialog-backroad-main-act");
                        ProgressSystemManager::Quests().SetStatus(World::JoeBackroadSideQuest, QuestStatus::OnGoing);
                        return;
                    }

                    if (isCowSpawned) {
                        // Джо сообщит, что искал её, но нашел
                        DialogSystemManager::StartDialog("Joe", "dialog-backroad-main-act-completed-before-start");
                        ProgressSystemManager::Quests().SetStatus(World::JoeBackroadSideQuest, QuestStatus::Completed);
                        return;
                    }
                }
                if (ProgressSystemManager::Quests().GetStatus(World::JoeBackroadSideQuest) == QuestStatus::OnGoing) {
                    if (!isCowSpawned) {
                        DialogSystemManager::StartDialog("Joe", "dialog-backroad-main-act-progress");
                        return;
                    }
                    if (isCowSpawned) {
                        DialogSystemManager::StartDialog("Joe", "dialog-backroad-main-act-after-quest-complete");
                        ProgressSystemManager::Quests().SetStatus(World::JoeBackroadSideQuest, QuestStatus::Completed);
                        return;
                    }
                }
                if (ProgressSystemManager::Quests().GetStatus(World::JoeBackroadSideQuest) == QuestStatus::Completed) {
                    DialogSystemManager::StartDialog("Joe", "dialog-backroad-main-act-resting");
                    return;
                }
            }

            if (mapName == GameplayMaps::OldHouse) {
                const int& joeOldHouseQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::JoeOldHouse, 9);
                DialogSystemManager::StartDialog("Joe", std::format("dialog-old-house-{}", joeOldHouseQuestProgress));
                return;
            }

            if (mapName == GameplayMaps::EldersHouse) {
                const int& joeEldersHouseQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::JoeEldersHouse, 14);
                DialogSystemManager::StartDialog("Joe", std::format("dialog-elders-house-{}", joeEldersHouseQuestProgress));
                return;
            }

            if (mapName == GameplayMaps::AssemblyHall) {
                const int& joeAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::JoeAssembly, 2);
                DialogSystemManager::StartDialog("Joe", std::format("dialog-assembly-hall-{}", joeAssemblyHallQuestProgress));
                return;
            }
        }

        /* COW DIALOGES */
        // Самые важные отношения с коровами в игре
        if (e.entityId == GameplayEntities::Cow) {
            auto const cowFeededQuestStatus = ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest);
            if (mapName == GameplayMaps::Crossroads) {
                auto const cowFeededQuestStatus = ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest);
                /* Засчитывание квеста на кормление если есть морковь */
                if (cowFeededQuestStatus == QuestStatus::NotStarted || cowFeededQuestStatus == QuestStatus::OnGoing) {
                    if (ProgressSystemManager::Inventory().HasItem(World::carrot)) {
                        DialogSystemManager::StartDialog("Cow", "dialog-cow-quest-carrot");
                        ProgressSystemManager::Inventory().RemoveItem(World::carrot);
                        ProgressSystemManager::Quests().SetStatus(World::CowFeededQuest, QuestStatus::Completed);
                        ProgressSystemManager::SaveData();
                        return;
                    }
                }
                /* Первая встреча с коровой — она дает квест */
                if (cowFeededQuestStatus == QuestStatus::NotStarted) {
                    DialogSystemManager::StartDialog("Cow", "dialog-cow-idle");
                    ProgressSystemManager::Quests().SetStatus(World::CowFeededQuest, QuestStatus::OnGoing);
                    return;
                }

                /* Вторая встреча с коровой — она дает прогресс квеста */
                if (cowFeededQuestStatus == QuestStatus::OnGoing) {
                    int const progress = (std::rand() % 3) + 1;
                    DialogSystemManager::StartDialog("Cow", std::format("dialog-cow-quest-carrot-progress-{}", progress));
                    return;
                }
                /* Третья встреча с коровой — она дает завершение квеста (счастливая корова) */
                if (cowFeededQuestStatus == QuestStatus::Completed) {
                    DialogSystemManager::StartDialog("Cow", "dialog-cow-quest-complete");
                    return;
                }
            }
            if (mapName == GameplayMaps::AssemblyHall) {
                if (ProgressSystemManager::Quests().GetStatus(World::CowFeededQuest) == QuestStatus::Completed) {
                    const int& cowAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::CowAssemblyHall, 2);
                    DialogSystemManager::StartDialog("Cow", std::format("dialog-cow-assembly-hall-{}-with-carrot", cowAssemblyHallQuestProgress));
                    return;
                } else {
                    const int& cowAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::CowAssemblyHall, 1);
                    DialogSystemManager::StartDialog("Cow", std::format("dialog-cow-assembly-hall-{}-no-carrot", cowAssemblyHallQuestProgress));
                    return;
                }
            }
        }

        /* CAT DIALOGES */
        if (e.entityId == GameplayEntities::Cat) {
            if (mapName == GameplayMaps::EldersHouse) {
                auto* cat = MapManager::GetEntitiesContainer().FindEntity({"cat", "vil"});
                if (cat) {
                    if (currentActiveQuestId == World::ElderCatQuest) {
                        if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::NotStarted) {
                            DialogSystemManager::StartDialog("Cat", "dialog-cat-run-away-1");
                            cat->SetPosition(-143, 88);
                            ProgressSystemManager::Quests().SetStatus(World::ElderCatQuest, QuestStatus::OnGoing);
                            return;
                        }
                        if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::OnGoing) {
                            DialogSystemManager::StartDialog("Cat", "dialog-cat-run-away-2");
                            cat->SetPosition(149, 88);
                            ProgressSystemManager::Quests().SetStatus(World::ElderCatQuest, QuestStatus::Completed);
                            return;
                        }
                        if (ProgressSystemManager::Quests().GetStatus(World::ElderCatQuest) == QuestStatus::Completed) {
                            DialogSystemManager::StartDialog("Cat", "dialog-cat-run-away-final");
                            return;
                        }
                    }
                }
            }
            if (mapName == GameplayMaps::AssemblyHall) {
                if (ProgressSystemManager::Player().catWasDestroyed) {
                    const int& catAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::CatAssemblyHall, 8);
                    DialogSystemManager::StartDialog("Cat", std::format("dialog-cat-assembly-hall-{}-no-cat", catAssemblyHallQuestProgress));
                    return;
                } else {
                    const int& catAssemblyHallQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::CatAssemblyHall, 2);
                    DialogSystemManager::StartDialog("Cat", std::format("dialog-cat-assembly-hall-{}-with-cat", catAssemblyHallQuestProgress));
                    return;
                }
                return;
            }
        }

        /* NEBULA DIALOGES */
        if (e.entityId == GameplayEntities::Nebula) {
            if (mapName == GameplayMaps::WorldVoid) {
                const int& nebulaQuestProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::Nebula, 7);
                DialogSystemManager::StartDialog("Nebula", std::format("nebula-dialog-{}", nebulaQuestProgress));
                return;
            }
        }

        if (e.entityId == GameplayEntities::ChestBox) {
            if (mapName == GameplayMaps::EldersHouse) {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::chestBoxElderHouse)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                    return;
                } else {
                    ProgressSystemManager::Inventory().SetChestBoxOpened(World::chestBoxElderHouse);
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-get-key");
                    ProgressSystemManager::Inventory().AddItem(World::key);
                    return;
                }
            }
            // Здесь добавляем книгу в инвентарь (нужна старейшине)
            if (mapName == GameplayMaps::OldHouse) {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::chestBoxOldHouse)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                    return;
                } else {
                    if (ProgressSystemManager::Inventory().HasItem(World::key)) {
                        ProgressSystemManager::Inventory().RemoveItem(World::key);
                        ProgressSystemManager::Inventory().AddItem(World::book);
                        ProgressSystemManager::Inventory().SetChestBoxOpened(World::chestBoxOldHouse);
                        DialogSystemManager::StartDialog("Utility", "dialog-chestbox-use-key");
                        return;
                    } else {
                        DialogSystemManager::StartDialog("Utility", "dialog-chestbox-no-key");
                        return;
                    }
                }
            }
        }

        if (e.entityId == GameplayEntities::Sword) {
            if (mapName == GameplayMaps::OldHouse) {
                if (ProgressSystemManager::Inventory().ChestBoxWasOpened(World::swordBox)) {
                    DialogSystemManager::StartDialog("Utility", "dialog-chestbox-already-opened");
                    return;
                } else {
                    ProgressSystemManager::Inventory().SetChestBoxOpened(World::swordBox);
                    ProgressSystemManager::Inventory().AddItem(World::sword);
                    DialogSystemManager::StartDialog("Utility", "dialog-get-sword");
                    return;
                }
            }
        }

        if (e.entityId == GameplayEntities::ExitTip) {
            if (mapName == GameplayMaps::AssemblyHall) {
                const int& exitTipVoidProgress = ProgressSystemManager::Player().AdvanceDialogProgress(DialogTrackIds::ExitTipVoid, 2);
                DialogSystemManager::StartDialog("Utility", std::format("dialog-exit-tip-void-{}", exitTipVoidProgress));
                return;
            }
        }
    }

    void OnEntityCreated(EntityCreatedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityCreated: {} {}", e.GetName(), e.GetType()));
        QuestId const currentActiveQuestId = ProgressSystemManager::Quests().GetCurrentActiveQuest();
        std::string const currentMapName = MapManager::GetCurrentMapName();
    }

    void OnEntityDestroyed(EntityDestroyedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
        if (e.GetName() == "haystack-1" || e.GetName() == "haystack-2") {
            auto const joeCarrotQuestStatus = ProgressSystemManager::Quests().GetStatus(World::JoeCarrotQuest);
            if (joeCarrotQuestStatus == QuestStatus::NotStarted) {
                ProgressSystemManager::Quests().SetStatus(World::JoeCarrotQuest, QuestStatus::OnGoing);
            }
            if (joeCarrotQuestStatus == QuestStatus::OnGoing) {
                ProgressSystemManager::Quests().SetStatus(World::JoeCarrotQuest, QuestStatus::Completed);
            }
        }

        if (e.GetName() == "cat") {
            /* IF PLAYER DELETE CAT IN ELDER'S HOUSE AND CAT QUEST IS ACTIVE */
            if (MapManager::GetCurrentMapName() == GameplayMaps::EldersHouse || MapManager::GetCurrentMapName() == GameplayMaps::Crossroads) {
                if (ProgressSystemManager::Quests().GetCurrentActiveQuest() == World::ElderCatQuest) {
                    ProgressSystemManager::Quests().SetStatus(World::ElderCatQuest, QuestStatus::Completed);
                    ProgressSystemManager::Quests().SetCurrentActiveQuest(World::ElderSpawnGuardQuest);
                    ProgressSystemManager::Player().catWasDestroyed = true; // Повлияет на концовку
                    DialogSystemManager::StartDialog("Elder", "dialog-spawn-guard-quest-after-cat-destroyed");
                    ProgressSystemManager::SaveData();
                    return;
                }
            }
        }
    }

    void OnDialogEnded(DialogEndedEvent& e) {
        Logger::Log(std::format("[Gameplay][Main] DialogEnded: {}", e.dialogId));
        if (e.dialogId == "nebula-dialog-7") {
            ProgressSystemManager::Player().gameEnded = true;
            ProgressSystemManager::SaveData();
        }
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