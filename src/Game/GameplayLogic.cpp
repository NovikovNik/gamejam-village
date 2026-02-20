#include "GameplayLogic.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <EventBus/EventBus.h>
#include <Events/EntitiesEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/LocationChangedEvent.h>
#include <Renderer/Camera.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/GameShutdownEvent.h>
#include <ProgressSystem/ProgressSystem.h>
#include <DialogSystem/DialogSystem.h>
#include <Map/Map.h>
#include <Entities/EInteractable.h>
#include <Entities/ENpc.h>
#include <memory>
#include <format>
#include <cmath>

namespace {
    class GameAct {
    public:
        virtual ~GameAct() = default;
    
        virtual void Initialize() {};
        // Апдейт для простых движений обьектов на карте
        virtual void Update(float deltaTime) {};
    };

    class ActIntro: public GameAct {
    public:
        void Initialize() {
            if (MapManager::GetCurrentMapName() != "intro") {
                Logger::Err("Intro act can only be loaded on intro map");
                return;
            }
            // Скейл 1 к 1, ибо это интро
            World::Camera::instance().SetScaleFactor(1.0f);

            onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &ActIntro::OnLocationChanged);
            onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &ActIntro::OnDialogEnded);
            EventBus::instance().EmitEvent<ForceDialogStartEvent>("Intro", "intro-1");
            
            penchamentEntity = MapManager::GetEntitiesContainer().FindEntity<World::EInteractable>();
            if (penchamentEntity) {
                baseBoxY = penchamentEntity->GetPosition().y;
                timeAccumulator = 0.0f;
            }
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
                EventBus::instance().EmitEvent<ChangeLocationEvent>("assets/maps/world-entry-2.json");
            }
        }

        void OnLocationChanged(LocationChangedEvent& e) {
            if (e.locationName == "world-entry-2") {
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

    // В этом акте мы первый раз идем в деревню и встречаем там старейшину
    class ActTutorial: public GameAct {
    public:
        void Initialize() {
            if (MapManager::GetCurrentMapName() != "world-entry-2") {
                Logger::Err("Tutorial act can only be loaded on world-entry-2 map");
                return;
            }
            // Скейл такой выверен на глаз
            World::Camera::instance().SetScaleFactor(1.7f);

            onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &ActTutorial::OnInteractWithEntity);
            onEntityCreated   = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &ActTutorial::OnEntityCreated);
            onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &ActTutorial::OnEntityDestroyed);

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
                ProgressSystemManager::Player().ResetToDefaults();
                EventBus::instance().EmitEvent<GameShutdownEvent>();
            }
        }

        void OnEntityDestroyed(EntityDestroyedEvent& e) {
            // Тут запускаем диалог после удаления старейшины
            Logger::Log(std::format("[Gameplay][Tutorial] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
            if (e.GetName() == "Elder") {
                // На случай если игрок попробует удалить старейшину пока идет диалог с ним
                // мы завершим диалог и запустим новый
                if (DialogSystemManager::IsDialogActive()) {
                    DialogSystemManager::EndDialog();
                }
                DialogSystemManager::StartDialog("Elder", "dialog-after-deleted");
            }
            if (e.GetName() == "Road_Sign") {
                // Игрок может удалить дорожный знак — он останется на карте, но это будет
                // обыграно словно он стер надпсь на нём
                Logger::Log("[Gameplay][Tutorial] Road_Sign destroyed");
                DialogSystemManager::StartDialog("Utility", "roadside-info-destroyed");
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
                DialogSystemManager::StartDialog("Elder", dialogId);
            }

            if (e.entityId == "Road_Sign") {
                Logger::Log("[Gameplay][Tutorial] Road_Sign interacted");
                DialogSystemManager::StartDialog("Utility", "roadside-info-1");
            }
        }

    private:
        Events::Handler onInteractWithEntity;
        Events::Handler onEntityCreated;
        Events::Handler onEntityDestroyed;

        int elderInteractionsCount = 0;
    };
    
    [[nodiscard]] std::unique_ptr<GameAct> CreateGameAct(GameActId id) {
        if (id == GameActIds::Intro) {
            return std::make_unique<ActIntro>();
        }
        if (id == GameActIds::Tutorial) {
            return std::make_unique<ActTutorial>();
        }
        return nullptr;
    }
}

class GameplayLogicManager: public Singleton<GameplayLogicManager> {
public:
    void Initialize() {
        const std::string& saved = ProgressSystemManager::Player().lastGameAct;
        GameActId id = GameActIds::Intro; // По умолчанию стартуем с intro

        // Если в сохранке есть другой акт, то загружаем его
        if (saved == GameActIds::Tutorial) {
            id = GameActIds::Tutorial;
        }

        nextGameAct = CreateGameAct(id);
        currentGameActId = std::string(id);
    }

    void LoadGameAct(GameActId id) {
        nextGameAct = CreateGameAct(id);
        currentGameActId = std::string(id);
        ProgressSystemManager::Player().lastGameAct = currentGameActId;
        ProgressSystemManager::SaveData();
    }

    std::string GetCurrentGameActId() const {
        return currentGameActId;
    }

    void Destroy() {
        if (gameAct != nullptr) {
            gameAct.reset();
        }
        if (nextGameAct != nullptr) {
            nextGameAct.reset();
        }
    }

    void UpdateCurrentGameAct() {
        if (nextGameAct != nullptr) {
            gameAct = std::move(nextGameAct);
            gameAct->Initialize();
            nextGameAct = nullptr;
        }
    }

    void Update(float deltaTime) {
        if (gameAct != nullptr) {
            gameAct->Update(deltaTime);
        }
    }

private:
    std::unique_ptr<GameAct> gameAct;
    std::unique_ptr<GameAct> nextGameAct;
    std::string currentGameActId;
};

namespace GameplayLogic {
    void Initialize() {
        GameplayLogicManager::instance().Initialize();
    }

    void LoadGameAct(GameActId id) {
        GameplayLogicManager::instance().LoadGameAct(id);
    }

    std::string GetCurrentGameActId() {
        return GameplayLogicManager::instance().GetCurrentGameActId();
    }

    void Destroy() {
        GameplayLogicManager::instance().Destroy();
    }

    void UpdateCurrentGameAct() {
        GameplayLogicManager::instance().UpdateCurrentGameAct();
    }

    void Update(float deltaTime) {
        GameplayLogicManager::instance().Update(deltaTime);
    }
}
