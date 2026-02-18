#include "EPlayer.h"
#include "EntityEventHandler.h"
#include "../Game/GameStates.h"
#include "../Game/GameFeatures.h"
#include "Entities/EColliders.h"
#include "Entities/EPit.h"
#include "Entities/ETriggerLocation.h"
#include <Map/Map.h>
#include <Entities/EBox.h>
#include <Entities/EInteractable.h>
#include <DialogSystem/DialogSystem.h>
#include <Logger/Logger.h>
#include <format>

World::EPlayer::EPlayer(const std::string& name, float x, float y) : name(name) {
    LoadData("player"_nnTex, x, y, 64, 64);
}

bool World::EPlayer::Update(float deltaTime) {
    
    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    if (DialogSystemManager::IsDialogActive()) {
        return false;
    }

    std::vector<World::EColliders*> colliders;
    MapManager::GetEntitiesContainer().FindEntities(colliders);

    std::vector<World::EBox*> boxes;
    MapManager::GetEntitiesContainer().FindEntities(boxes);

    std::vector<World::EInteractable*> interactibles;
    MapManager::GetEntitiesContainer().FindEntities(interactibles);

    std::vector<World::EPit*> pits;
    MapManager::GetEntitiesContainer().FindEntities(pits);

    std::vector<World::ETriggerLocation*> levelChangeTriggers;
    MapManager::GetEntitiesContainer().FindEntities(levelChangeTriggers);

    glm::vec2 direction(0.0f);

     if (GameStates::instance().w) {
         direction.y -= 1.0f;
     }
     if (GameStates::instance().s) {
         direction.y += 1.0f;
     }
     if (GameStates::instance().a) {
         direction.x -= 1.0f;
     }
     if (GameStates::instance().d) {
         direction.x += 1.0f;
     }

     if (glm::length(direction) > 0.0f) {
         direction = glm::normalize(direction);

         glm::vec2 pos = GetPosition();
         float moveX = direction.x * basicSpeed * deltaTime;
         float moveY = direction.y * basicSpeed * deltaTime;

         const float halfW = GetWidth() * 0.5f;
         const float halfH = GetHeight() * 0.5f;

         auto WouldCollide = [&](float dx, float dy) {
             const float pl = pos.x + dx - halfW;
             const float pr = pos.x + dx + halfW;
             const float pt = pos.y + dy - halfH;
             const float pb = pos.y + dy + halfH;

             auto overlaps = [&](float l, float r, float t, float b) {
                 return pl < r && pr > l && pt < b && pb > t;
             };

             for (auto* collider : colliders) {
                 for (const auto& c : collider->GetColliders()) {
                     if (overlaps(c.x - c.width * 0.5f, c.x + c.width * 0.5f, c.y - c.height * 0.5f, c.y + c.height * 0.5f))
                         return true;
                 }
             }
             for (auto* e : interactibles) {
                 if (!e->IsValid()) continue;
                 const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
                 const glm::vec2 p = e->GetPosition();
                 if (overlaps(p.x - w2, p.x + w2, p.y - h2, p.y + h2)) return true;
             }
             for (auto* e : pits) {
                 if (!e->IsValid()) continue;
                 const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
                 const glm::vec2 p = e->GetPosition();
                 if (overlaps(p.x - w2, p.x + w2, p.y - h2, p.y + h2)) return true;
             }
             for (auto* box : boxes) {
                 if (!box->IsValid()) continue;
                 const float w2 = box->GetWidth() * 0.5f, h2 = box->GetHeight() * 0.5f;
                 const glm::vec2 p = box->GetPosition();
                 if (overlaps(p.x - w2, p.x + w2, p.y - h2, p.y + h2)) {
                     const float pushX = (dx != 0.0f) ? (dx > 0.0f ? 1.0f : -1.0f) : 0.0f;
                     const float pushY = (dy != 0.0f) ? (dy > 0.0f ? 1.0f : -1.0f) : 0.0f;
                     if (!box->CanMove(pushX, pushY, basicSpeed, deltaTime)) return true;
                 }
             }
             for (auto* e : levelChangeTriggers) {
                 if (!e->IsValid()) continue;
                 const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
                 const glm::vec2 p = e->GetPosition();
                 if (overlaps(p.x - w2, p.x + w2, p.y - h2, p.y + h2)) {
                     e->ChangeLocation();
                     return true;
                 }
             }
             return false;
         };

         float impulseX = (WouldCollide(moveX, 0) ? 0.0f : direction.x) * basicSpeed;
         float impulseY = (WouldCollide(0, moveY) ? 0.0f : direction.y) * basicSpeed;

         AddImpulse(impulseX, impulseY);
         if (impulseX != 0.0f || impulseY != 0.0f) {
             OnMoved(deltaTime);
         }
     }

     EInteractable* interactable = TryInteract();
     if (interactable) {
        bShowTooltip = true;
     } else {
        bShowTooltip = false;
     }

     return true;
 }

 void World::EPlayer::OnInterectButtonPressed(::InterectButtonPressedEvent& event) {
    if (DialogSystemManager::IsDialogActive()) {
        Logger::Warn("Dialog is active, skipping interact");
        return;
    }
    EInteractable* interactable = TryInteract();
    Logger::Log(std::format("Interactable: {}", interactable ? "true" : "false"));
    if (interactable) {
        interactable->Interact();
    }
 }

 void World::EPlayer::OnSpawn() {
    EntityEventHandler::Subscribe<InterectButtonPressedEvent>(this, &EPlayer::OnInterectButtonPressed);
    tooltipTexture = make_nnTex("f_button");
}

void World::EPlayer::SetTooltipTexture(Renderer::TextureId texture, float w, float h) {
    tooltipTexture = texture;
    tooltipWidth = w;
    tooltipHeight = h;
}

void World::EPlayer::OnMoved(float deltaTime) {
    std::vector<World::EBox*> boxes;
    MapManager::GetEntitiesContainer().FindEntities(boxes);
    for (const auto& box : boxes) {
        const auto boxPosition = box->GetPosition();
        const auto playerPosition = GetPosition();
        const auto delta = boxPosition - playerPosition;

        const auto minDistance = 64.f;
        if (abs(delta.y) > abs(delta.x)) {
            if (delta.y > 0 && delta.y < minDistance / 1.1f) {
                box->Move(0, 1, basicSpeed, deltaTime);
            }
            else if (delta.y < 0 && delta.y > -minDistance / 1.5f) {
                box->Move(0, -1, basicSpeed, deltaTime);
            }
        }
        else {
            if (delta.x > 0 && delta.x < minDistance / 1.5f) {
                box->Move(1, 0, basicSpeed, deltaTime);
            }
            else if (delta.x < 0 && delta.x > -minDistance / 1.5f) {
                box->Move(-1, 0, basicSpeed, deltaTime);
            }
        }
    }
}

void World::EPlayer::Render(float deltaTime) {
    EMovable::Render(deltaTime);
    if (GameFeatures::isDebug) {
        Renderer::DrawRectangle(GetPosition().x, GetPosition().y, GetWidth(), GetHeight(), 0.0);
    }
    if (bShowTooltip && !DialogSystemManager::IsDialogActive()) {
        const auto& pos = GetPosition();
        Renderer::DrawSprite(tooltipTexture, pos.x, pos.y - tooltipHeight - 16.0f, tooltipWidth, tooltipHeight);
    }
}

World::EInteractable* World::EPlayer::TryInteract() const {
    std::vector<World::EInteractable*> interactables;
    MapManager::GetEntitiesContainer().FindEntities(interactables);
    for (const auto& interactable : interactables) {
        const auto interactablePosition = interactable->GetPosition();
        const auto interactableWidthWithOffset = interactable->GetWidth() + 20.0f; // 20.0f то оффсет велью вывереный на глаз
        const auto playerPosition = GetPosition();
        const auto distance = glm::distance(interactablePosition, playerPosition);
        if (distance < interactableWidthWithOffset) {
            return interactable;
        }
    }
    return nullptr;
}
