#include "GameplayLogic.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <EventBus/EventBus.h>
#include <Events/EntitiesEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/GameShutdownEvent.h>
#include <ProgressSystem/ProgressSystem.h>
#include <Map/Map.h>
#include <memory>
#include <format>

namespace {
    class GameAct {
    public:
        virtual ~GameAct() = default;
    
        virtual void Initialize() {};
    };

    class ActIntro: public GameAct {
    public:
        void Initialize() {
            if (MapManager::GetCurrentMapName() != "intro") {
                Logger::Err("Intro act can only be loaded on intro map");
                return;
            }
            onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &ActIntro::OnDialogEnded);
            EventBus::instance().EmitEvent<ForceDialogStartEvent>("Intro", "intro-1");
        }

        void OnDialogEnded(DialogEndedEvent& e) {
            Logger::Log(std::format("[Gameplay][Intro] DialogEnded: {}", e.dialogId));
            if (e.characterId == "Intro" && e.dialogId == "intro-1") {
                ::GameplayLogic::LoadGameAct("tutorial");
                EventBus::instance().EmitEvent<ChangeLocationEvent>("assets/maps/world-entry-2.json");
            }
        }

    private:
        Events::Handler onDialogEnded;
    };

    // В этом акте мы первый раз идем в деревню и встречаем там старейшину
    class ActTutorial: public GameAct {
    public:
        void Initialize() {
            if (MapManager::GetCurrentMapName() != "world-entry-2") {
                Logger::Err("Tutorial act can only be loaded on world-entry-2 map");
                return;
            }
            onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &ActTutorial::OnInteractWithEntity);
            onEntityCreated   = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &ActTutorial::OnEntityCreated);
            onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &ActTutorial::OnEntityDestroyed);
        }

        void OnEntityCreated(EntityCreatedEvent& e) {
            Logger::Log(std::format("[Gameplay][Tutorial] EntityCreated: {} {}", e.GetName(), e.GetType()));
            // Здесь мы полностью сбрасываем состояние игры по приколу! Поэтому здесь и очистка сохранения должна быть!
            if (e.GetName() == "kick-my" && e.GetType() == "ass") {
                Logger::Log("[Gameplay][Tutorial] Kick my ass file created. Stopping the game.");
                ProgressSystemManager::Player().ResetToDefaults();
                EventBus::instance().EmitEvent<GameShutdownEvent>();
            }
        }

        void OnEntityDestroyed(EntityDestroyedEvent& e) {
            // Тут запускаем диалог после удаления старейшины
            Logger::Log(std::format("[Gameplay][Tutorial] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
            if (e.GetName() == "Elder") {
                EventBus::instance().EmitEvent<ForceDialogStartEvent>("Elder", "dialog-after-deleted");
            }
        }

        // Последний диалог dialog-8 (про kick-my-ass)
        void OnInteractWithEntity(InteractWithEntityEvent& e) {
            Logger::Log(std::format("[Gameplay][Tutorial] InteractWithEntity: {}", e.entityId));
            if (e.entityId == "Elder") {
                if (elderInteractionsCount < 8) {
                    elderInteractionsCount++;
                }
                std::string dialogId = std::format("dialog-{}", std::to_string(elderInteractionsCount));
                EventBus::instance().EmitEvent<ForceDialogStartEvent>("Elder", dialogId);
            }
        }

    private:
        Events::Handler onInteractWithEntity;
        Events::Handler onEntityCreated;
        Events::Handler onEntityDestroyed;

        int elderInteractionsCount = 0;
    };
    
    [[nodiscard]] std::unique_ptr<GameAct> CreateGameAct(const std::string& gameActName) {
        if (gameActName == "intro") {
            return std::make_unique<ActIntro>();
        }
        if (gameActName == "tutorial") {
            return std::make_unique<ActTutorial>();
        }
        return nullptr;
    }
}

class GameplayLogicManager: public Singleton<GameplayLogicManager> {
public:
    void Initialize() {
        nextGameAct = CreateGameAct("intro");
    }

    void LoadGameAct(const std::string& gameActName) {
        nextGameAct = CreateGameAct(gameActName); // А правильно ли?
    }

    void Destroy() {
        if (gameAct != nullptr) {   
            gameAct.reset();
        }
        if (nextGameAct != nullptr) {
            nextGameAct.reset();
        }
    }

    void Update() {
        if (nextGameAct != nullptr) {
            gameAct = std::move(nextGameAct);
            gameAct->Initialize();
            nextGameAct = nullptr;
        }
    }

private:
    std::unique_ptr<GameAct> gameAct;
    std::unique_ptr<GameAct> nextGameAct;
};

namespace GameplayLogic {
    void Initialize() {
        GameplayLogicManager::instance().Initialize();
    }

    void LoadGameAct(const std::string& gameActName) {
        GameplayLogicManager::instance().LoadGameAct(gameActName);
    }

    void Destroy() {
        GameplayLogicManager::instance().Destroy();
    }

    void Update() {
        GameplayLogicManager::instance().Update();
    }
}
