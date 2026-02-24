#pragma once

#include "GameActBase.h"
#include <Events/InteractWithEntityEvent.h>
#include <Events/InterectButtonPressedEvent.h>
#include <Events/LocationChangedEvent.h>
#include <EventBus/EventBus.h>
#include <DialogSystem/DialogSystem.h>
#include <Gameplay/WorldState.h>
#include <string>
#include <vector>
#include <format>

namespace GameActs {

class SignsAct : public GameAct {
public:
    void Initialize() {
        onInteractWithEntity = EventBus::instance().SubscribeToEvent<InteractWithEntityEvent>(this, &SignsAct::OnInteractWithEntity);
        onInterectButtonPressed = EventBus::instance().SubscribeToEvent<InterectButtonPressedEvent>(this, &SignsAct::OnInterectButtonPressed);
        onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(this, &SignsAct::OnLocationChanged);
    }

    void Update(float /*deltaTime*/) override {}

    void OnInteractWithEntity(InteractWithEntityEvent& e) {
        if (DialogSystemManager::IsDialogActive()) {
            return;
        }
        if (e.entityId == "Name_Sign") {
            const auto& registeredEntities = WorldState::GetCurrentState().registeredEntities;
            std::vector<std::string> signRows;
            for (const auto& [key, locations] : registeredEntities) {
                if (locations.empty() && key.find("vil") != std::string::npos && WorldState::GetEntitiesWhiteList().contains(key)) {
                    signRows.push_back(key);
                }
            }
            signRows.push_back("***");
            DialogSystemManager::OpenSign(signRows);
        }

        if (e.entityId == "Locations_Sign") {
            const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
            std::vector<std::string> signRows;
            for (const auto& [key, available] : registeredLocations) {
                if (!available && WorldState::GetLocationsWhiteList().contains(key)) {
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

    void OnLocationChanged(LocationChangedEvent& /*e*/) {
        DialogSystemManager::CloseSign();
        DialogSystemManager::EndDialog();
    }

private:
    Events::Handler onInteractWithEntity;
    Events::Handler onInterectButtonPressed;
    Events::Handler onLocationChanged;
};

} // namespace GameActs