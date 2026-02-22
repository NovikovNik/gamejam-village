#include "GameplayLogic.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <EventBus/EventBus.h>
#include <Events/EntitiesEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/ChangeLocationEvent.h>
#include <Events/LocationChangedEvent.h>
#include <Events/InterectButtonPressedEvent.h>
#include <Renderer/Camera.h>
#include <Events/InteractWithEntityEvent.h>
#include <Events/GameShutdownEvent.h>
#include <Gameplay/WorldState.h>
#include <ProgressSystem/ProgressSystem.h>
#include <DialogSystem/DialogSystem.h>
#include <Map/Map.h>
#include <Entities/EInteractable.h>
#include <Entities/ENpc.h>
#include <memory>
#include <format>
#include <cmath>
#include <vector>

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

    // В этом акте мы первый раз идем в деревню и встречаем там старейшину
    class ActTutorial: public GameAct {
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
                ProgressSystemManager::Player().ResetToDefaults();
                EventBus::instance().EmitEvent<GameShutdownEvent>();
            }
        }

        void OnEntityDestroyed(EntityDestroyedEvent& e) {
            // Тут запускаем диалог после удаления старейшины
            Logger::Log(std::format("[Gameplay][Tutorial] EntityDestroyed: {} {}", e.GetName(), e.GetType()));
            if (e.GetName() == "Guard") {
                // На случай если игрок попробует удалить старейшину пока идет диалог с ним
                // мы завершим диалог и запустим новый
                if (DialogSystemManager::IsDialogActive()) {
                    DialogSystemManager::EndDialog();
                }
                DialogSystemManager::StartDialog("Guard", "dialog-after-deleted");
            }
            if (e.GetName() == "Road_Sign") {
                // Игрок может удалить дорожный знак — он останется на карте, но это будет
                // обыграно словно он стер надпсь на нём
                Logger::Log("[Gameplay][Tutorial] Road_Sign destroyed");
                DialogSystemManager::StartDialog("Utility", "roadside-info-destroyed");
            }
        }

        void OnDialogEnded(DialogEndedEvent& e) {
            Logger::Log(std::format("[Gameplay][Intro] DialogEnded: {}", e.dialogId));
            if (e.characterId == "Guard" && e.dialogId == "dialog-1") {
                ProgressSystemManager::Player().fileSystemIconVisible = true;
                ProgressSystemManager::SaveData();
            }
        }

        // Последний диалог dialog-8 (про kick-my-ass)
        void OnInteractWithEntity(InteractWithEntityEvent& e) {
            Logger::Log(std::format("[Gameplay][Tutorial] InteractWithEntity: {}", e.entityId));
            if (e.entityId == "Guard") {
                if (guardInteractionsCount < 8) {
                    guardInteractionsCount++;
                }
                std::string dialogId = std::format("dialog-{}", std::to_string(guardInteractionsCount));
                DialogSystemManager::StartDialog("Guard", dialogId);
            }

            if (e.entityId == "Road_Sign") {
                Logger::Log("[Gameplay][Tutorial] Road_Sign interacted");
                DialogSystemManager::StartDialog("Utility", "roadside-info-1");
            }
        }
    
        void OnLocationChanged(LocationChangedEvent& e) {
            Logger::Log(std::format("[Gameplay][Tutorial] LocationChanged: {}", e.locationName));
            if (e.locationName == "crossroads") {
                ::GameplayLogic::LoadGameAct(GameActIds::Main);
            }
        }

    private:
        Events::Handler onInteractWithEntity;
        Events::Handler onEntityCreated;
        Events::Handler onEntityDestroyed;
        Events::Handler onDialogEnded;
        Events::Handler onLocationChanged;

        int guardInteractionsCount = 0;
    };

    /* Основной акт, который может быть запущен несколько раз. В основном работает в хабе игры */
    class MainAct: public GameAct {
    public:
        void Initialize() {
            onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);
            onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &MainAct::OnInteractWithEntity);
            onEntityCreated = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &MainAct::OnEntityCreated);
            onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &MainAct::OnEntityDestroyed);
            onDialogEnded = EventBus::instance().SubscribeToEvent<DialogEndedEvent>(this, &MainAct::OnDialogEnded);
            onLocationChangedEvent = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &MainAct::OnLocationChanged);
        }
        
        void Update(float deltaTime) override {}

        void OnLocationChanged(LocationChangedEvent& e) {
            Logger::Log(std::format("[Gameplay][Main] LocationChanged: {}", e.locationName));
            if (e.locationName == "backroad") {
                locationChanged = true;
            }
        }

        void OnInteractWithEntity(InteractWithEntityEvent& e) {
            Logger::Log(std::format("[Gameplay][Main] InteractWithEntity: {}", e.entityId));
            std::string const mapName = MapManager::GetCurrentMapName();

            if (e.entityId == "Elder") {
                // Первый диалог со старейшиной в деревне. Он заберет письмо и попросит пройтись через деревню
                if (mapName == "crossroads") {
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
                        auto it = registeredEntities.find("Cat.vil");
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
                        auto it = registeredEntities.find("Guard.vil");
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
                }
            }

            if (e.entityId == "Guard") {
                if (mapName == "crossroads") {
                    if (ProgressSystemManager::Player().elderHubActiveQuest == "guard_quest") {
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
                }
            }
        }

        void OnEntityCreated(EntityCreatedEvent& e) {
            Logger::Log(std::format("[Gameplay][Main] EntityCreated: {} {}", e.GetName(), e.GetType()));
            if (MapManager::GetCurrentMapName() == "elders-house") {
                if (e.GetName() == "Cat") {
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
    };
    
    [[nodiscard]] std::unique_ptr<GameAct> CreateGameAct(GameActId id) {
        if (id == GameActIds::Intro) {
            return std::make_unique<ActIntro>();
        }
        if (id == GameActIds::Tutorial) {
            return std::make_unique<ActTutorial>();
        }
        if (id == GameActIds::Main) {
            return std::make_unique<MainAct>();
        }
        return nullptr;
    }
}

/* Основной акт, который может быть запущен несколько раз. В основном работает в хабе игры */
class SignsAct: public GameAct {
    public:
        void Initialize() {
            onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &SignsAct::OnInteractWithEntity);
            onInterectButtonPressed = EventBus::instance().SubscribeToEvent<InterectButtonPressedEvent>(this, &SignsAct::OnInterectButtonPressed);
        }
        
        void Update(float deltaTime) override {}

        void OnInteractWithEntity(InteractWithEntityEvent& e) {
            if (DialogSystemManager::IsDialogActive()) {
                return;
            }
            if (e.entityId == "Name_Sign") {
                const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
                std::vector<std::string> signRows;
                for (const auto& [key, locations] : registeredEntities) {
                    if (locations.empty() && key.find("vil") != std::string::npos) {
                        signRows.push_back(key);
                    }
                }
                DialogSystemManager::OpenSign(signRows);
            }
            
            if (e.entityId == "Locations_Sign") {
                const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
                std::vector<std::string> signRows;
                for (const auto& [key, available] : registeredLocations) {
                    if (!available) {
                        signRows.push_back(key);
                    }
                }
                DialogSystemManager::OpenSign(signRows);
            }
        }

        void OnInterectButtonPressed(InterectButtonPressedEvent& e) {
            if (DialogSystemManager::IsDialogActive()) {
                DialogSystemManager::CloseSign();
            }
        }

    private:
        Events::Handler onInteractWithEntity;
        Events::Handler onInterectButtonPressed;
};

class GameplayLogicManager: public Singleton<GameplayLogicManager> {
public:
    void Initialize() {
        const std::string& saved = ProgressSystemManager::Player().lastGameAct;
        GameActId id = GameActIds::Intro; // По умолчанию стартуем с intro

        // Если в сохранке есть другой акт, то загружаем его
        if (saved == GameActIds::Tutorial) {
            id = GameActIds::Tutorial;
        }
        if (saved == GameActIds::Main) {
            id = GameActIds::Main;
        }

        nextGameAct = CreateGameAct(id);
        currentGameActId = std::string(id);

        currentGameAct = std::make_unique<SignsAct>();
        currentGameAct->Initialize();
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
        if (currentGameAct != nullptr) {
            currentGameAct->Update(deltaTime);
        }
    }

private:
    std::unique_ptr<GameAct> gameAct;
    std::unique_ptr<GameAct> nextGameAct;
    std::unique_ptr<GameAct> currentGameAct;
    std::string currentGameActId;

    Events::Handler onInteractWithEntity;
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
