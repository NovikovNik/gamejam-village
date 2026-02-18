#include "GameplayLogic.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <EventBus/EventBus.h>
#include <Events/EntitiesEvent.h>
#include <memory>
#include <format>

namespace {
    class GameAct {
    public:
        virtual ~GameAct() = default;
    
        virtual void Initialize() {};
    };

    class ActTutorial: public GameAct {
    public:
        void Initialize() {
            onEntityCreated   = EventBus::instance().SubscribeToEvent<EntityCreatedEvent>(this, &ActTutorial::OnEntityCreated);
            onEntityDestroyed = EventBus::instance().SubscribeToEvent<EntityDestroyedEvent>(this, &ActTutorial::OnEntityDestroyed);
        }

        void OnEntityCreated(EntityCreatedEvent& e) {
            Logger::Log(std::format("EntityCreated: {} {}", e.GetName(), e.GetType()));
        }

        void OnEntityDestroyed(EntityDestroyedEvent& e) {
            Logger::Log(std::format("EntityDestroyed: {} {}", e.GetName(), e.GetType()));
        }

    private:
        Events::Handler onEntityCreated;
        Events::Handler onEntityDestroyed;
    };
    
    [[nodiscard]] std::unique_ptr<GameAct> CreateGameAct(const std::string& gameActName) {
        if (gameActName == "tutorial") {
            return std::make_unique<ActTutorial>();
        }
        return nullptr;
    }
}

class GameplayLogicManager: public Singleton<GameplayLogicManager> {
public:
    void Initialize() {
        nextGameAct = CreateGameAct("tutorial");
    }

    void LoadGameAct(const std::string& gameActName) {

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

    void Destroy() {
        GameplayLogicManager::instance().Destroy();
    }

    void Update() {
        GameplayLogicManager::instance().Update();
    }
}
