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
#include <Game/GameActs/GameActBase.h>
#include <Game/GameActs/GameActIntro.h>
#include <Game/GameActs/GameActTutorial.h>
#include <Game/GameActs/GameActMain.h>
#include <Game/GameActs/GameActSign.h>
#include <memory>
#include <format>
#include <cmath>
#include <vector>

namespace {

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

        currentGameAct = std::make_unique<GameActs::SignsAct>();
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

    [[nodiscard]] std::unique_ptr<GameActs::GameAct> CreateGameAct(GameActId id) {
        if (id == GameActIds::Intro) {
            return std::make_unique<GameActs::ActIntro>();
        }
        if (id == GameActIds::Tutorial) {
            return std::make_unique<GameActs::ActTutorial>();
        }
        if (id == GameActIds::Main) {
            return std::make_unique<GameActs::MainAct>();
        }
        return nullptr;
    }

private:
    std::unique_ptr<GameActs::GameAct> gameAct;
    std::unique_ptr<GameActs::GameAct> nextGameAct;
    std::unique_ptr<GameActs::GameAct> currentGameAct;
    std::string currentGameActId;

    Events::Handler onInteractWithEntity;
    };
}

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
