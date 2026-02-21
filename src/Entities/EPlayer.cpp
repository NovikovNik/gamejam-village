#include "EPlayer.h"
#include "EntityEventHandler.h"
#include "../Game/GameStates.h"
#include <Events/PlaySoundEvent.h>
#include "../Game/GameFeatures.h"
//#include "Entities/EColliders.h"
#include "Entities/EPit.h"
#include "Entities/ETriggerLocation.h"
#include <Renderer/Camera.h>
#include <Map/Map.h>
#include <Entities/EBox.h>
#include <Entities/EInteractable.h>
#include <DialogSystem/DialogSystem.h>
#include <Logger/Logger.h>
#include <Gameplay/WorldState.h>
#include <format>
#include <optional>

namespace
{
    constexpr float FOOTSTEP_INTERVAL = 0.35f;
}

World::EPlayer::EPlayer(const std::string& name) : name(name) {
    LoadData("player"_nnTex, 64, 64);
    footstepTimer = FOOTSTEP_INTERVAL;

    animationIdle.textureId = make_nnTex("player_idle");
    animationIdle.numOfFrames = 5;
    animationIdle.maxElementsPerRow = 5;
    animationIdle.frameSize = 64;
    animationIdle.frameDelay = 0.15f;

    animationMoving.textureId = make_nnTex("player_moving");
    animationMoving.numOfFrames = 7;
    animationMoving.maxElementsPerRow = 7;
    animationMoving.frameSize = 64;
    animationMoving.frameDelay = 0.1f;
}

bool World::EPlayer::Update(float deltaTime) {

    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    const auto& movableObjectPositions = Physics::GetMovableObjectPositions();
    for (const auto& movableObjectPosition : movableObjectPositions) {
        if (movableObjectPosition.id == physicsObjectId) {
            SetPosition(movableObjectPosition.x, movableObjectPosition.y);
            break;
        }
    }

    if (DialogSystemManager::IsDialogActive()) {
        return false;
    }

//    std::vector<World::EColliders*> colliders;
//    MapManager::GetEntitiesContainer().FindEntities(colliders);

    std::vector<World::EBox*> boxes;
    MapManager::GetEntitiesContainer().FindEntities(boxes);

    std::vector<World::EInteractable*> interactibles;
    MapManager::GetEntitiesContainer().FindEntities(interactibles);

    std::vector<World::EPit*> pits;
    MapManager::GetEntitiesContainer().FindEntities(pits);

    std::vector<World::ETriggerLocation*> levelChangeTriggers;
    MapManager::GetEntitiesContainer().FindEntities(levelChangeTriggers);

    glm::vec2 inputDir(0.0f);

    if (GameStates::instance().w) {
        inputDir.y -= 1.0f;
    }
    if (GameStates::instance().s) {
        inputDir.y += 1.0f;
    }
    if (GameStates::instance().a) {
        inputDir.x -= 1.0f;
        horizontalFlip = true;
    }
    if (GameStates::instance().d) {
        inputDir.x += 1.0f;
        horizontalFlip = false;
    }

    if (glm::length(inputDir) > 0.0f) {
        direction = glm::normalize(inputDir);

        glm::vec2 pos = GetPosition();
        const float velocityX = direction.x * basicSpeed;
        const float velocityY = direction.y * basicSpeed;

//        const float halfW = GetWidth() * 0.5f;
//        const float halfH = GetHeight() * 0.5f;

//        auto overlaps = [&](float dx, float dy, float l, float r, float t, float b) {
//            const float pl = pos.x + dx - halfW;
//            const float pr = pos.x + dx + halfW;
//            const float pt = pos.y + dy - halfH;
//            const float pb = pos.y + dy + halfH;
//            return pl < r && pr > l && pt < b && pb > t;
//        };

//        auto ComputeImpulse = [&]() -> std::optional<glm::vec2> {
//            bool blockedX = false;
//            bool blockedY = false;
//
//            auto checkBoth = [&](float l, float r, float t, float b) {
//                if (overlaps(moveX, 0.0f, l, r, t, b)) { blockedX = true; }
//                if (overlaps(0.0f, moveY, l, r, t, b)) { blockedY = true; }
//            };
//
//            for (auto* collider : colliders) {
//                for (const auto& c : collider->GetColliders()) {
//                    checkBoth(c.x - c.width * 0.5f, c.x + c.width * 0.5f, c.y - c.height * 0.5f, c.y + c.height * 0.5f);
//                }
//            }
//
//            for (auto* e : interactibles) {
//                if (!e->IsValid()) { 
//                    continue; 
//                }
//                const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
//                const glm::vec2 p = e->GetPosition();
//                checkBoth(p.x - w2, p.x + w2, p.y - h2, p.y + h2);
//            }
//
//            for (auto* e : pits) {
//                if (!e->IsValid()) { 
//                    continue;
//                }
//                const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
//                const glm::vec2 p = e->GetPosition();
//                checkBoth(p.x - w2, p.x + w2, p.y - h2, p.y + h2);
//            }
//
//            for (auto* box : boxes) {
//                if (!box->IsValid()) { continue; }
//                const float w2 = box->GetWidth() * 0.5f, h2 = box->GetHeight() * 0.5f;
//                const glm::vec2 p = box->GetPosition();
//                bool isOverlaps = false;
//                if (overlaps(moveX, 0.0f, p.x - w2, p.x + w2, p.y - h2, p.y + h2)) {
//                    isOverlaps = true;
//                    if (!box->CanMove(moveX > 0.0f ? 1.0f : -1.0f, 0.0f, basicSpeed, deltaTime)) { 
//                        blockedX = true; 
//                    }
//                }
//                if (overlaps(0.0f, moveY, p.x - w2, p.x + w2, p.y - h2, p.y + h2)) {
//                    isOverlaps = true;
//                    if (!box->CanMove(0.0f, moveY > 0.0f ? 1.0f : -1.0f, basicSpeed, deltaTime)) { 
//                        blockedY = true; 
//                    }
//                }
//                if (isOverlaps) {
//                    WorldState::RegisterInWorldState(box->GetTagName());
//                }
//            }
//
//            for (auto* e : levelChangeTriggers) {
//                if (!e->IsValid()) { 
//                    continue; 
//                }
//                const float w2 = e->GetWidth() * 0.5f, h2 = e->GetHeight() * 0.5f;
//                const glm::vec2 p = e->GetPosition();
//                if (overlaps(moveX, 0.0f, p.x - w2, p.x + w2, p.y - h2, p.y + h2) ||
//                    overlaps(0.0f, moveY, p.x - w2, p.x + w2, p.y - h2, p.y + h2)) {
//                    e->ChangeLocation();
//                    return std::nullopt;
//                }
//            }
//
//            return glm::vec2{
//                blockedX ? 0.0f : direction.x * basicSpeed,
//                blockedY ? 0.0f : direction.y * basicSpeed
//            };
//        };
//
//        const auto impulse = ComputeImpulse();
//        if (!impulse) { return true; }

//        AddImpulse(impulse->x, impulse->y);


        if (velocityX != 0.0f || velocityY != 0.0f) {
            Physics::AddImpulse(physicsObjectId, velocityX, velocityY);
        }
    } else {
        direction = glm::vec2(0.0f);
        // Not moving — reset so the next step plays immediately when movement starts.
        footstepTimer = FOOTSTEP_INTERVAL;
    }

     /* Вот эта проверка тяжелая, но без неё происходит сегфолт, т.к во время проверки мы можем удалить NPC и когда
     дело дойдет сюда, то мы попытаемся вызвать IsValid() на уже удаленном NPC. По идее, т.к у нас не так много обьектов
     эта проверка будет не так затратна, но концептуально выглядит плохо */
     if (currentInteractable) {
        const auto& container = MapManager::GetEntitiesContainer();
        if (!container.Contains(currentInteractable)) {
            currentInteractable = nullptr;
            bShowTooltip = false;
        } else if (!currentInteractable->IsValid()) {
            currentInteractable = nullptr;
            bShowTooltip = false;
        }
     }
     EInteractable* interactable = TryInteract();
     if (interactable != currentInteractable) {
        currentInteractable = interactable;
        if (currentInteractable) {
            bShowTooltip = true;
            WorldState::RegisterInWorldState(currentInteractable->GetTagName());
        } else {
            bShowTooltip = false;
        }
     }

     return true;
 }

 void World::EPlayer::OnInterectButtonPressed(::InterectButtonPressedEvent& event) {
    if (DialogSystemManager::IsDialogActive()) {
        Logger::Warn("Dialog is active, skipping interact");
        return;
    }
    EInteractable* interactable = TryInteract();
    Logger::Log(std::format("[EPlayer] Interactable: {}", interactable ? "true" : "false"));
    if (interactable) {
        EventBus::instance().EmitEvent<PlaySoundEvent>("interact");
        interactable->Interact();
    }
 }

 void World::EPlayer::OnSpawn(float x, float y, float w, float h) {
    EMovable::OnSpawn(x, y, w, h);
    onInterectButtonPressed = EventBus::instance().SubscribeToEvent<InterectButtonPressedEvent>(this, &EPlayer::OnInterectButtonPressed);
    tooltipTexture = make_nnTex("f_button");

    physicsObjectId = Physics::CreateDynamicRectangle(x, y, GetWidth() * 0.5f, GetHeight() * 0.9f, 10.f, (1 << 0) | (1 << 1) | (1 << 2));
}

void World::EPlayer::SetTooltipTexture(Renderer::TextureId texture, float w, float h) {
    tooltipTexture = texture;
    tooltipWidth = w;
    tooltipHeight = h;
}

void World::EPlayer::OnMoved(float deltaTime) {
    footstepTimer += deltaTime;
    if (footstepTimer >= FOOTSTEP_INTERVAL) {
        footstepTimer = 0.0f;
        EventBus::instance().EmitEvent<PlaySoundEvent>("footstep");
    }

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
    // Система определения движения: direction обновляется в Update(); при вводе — длина > 0, иначе 0.
    const bool isMoving = glm::length(direction) > 0.0f;
    if (isMoving) {
        Renderer::RenderAnimation(animationMoving, deltaTime, positionX, positionY, width, height, horizontalFlip);
    } else {
        Renderer::RenderAnimation(animationIdle, deltaTime, positionX, positionY, width, height, horizontalFlip);
    }

//    if (GameFeatures::isDebug) {
//        const auto& pos = GetPosition();
//        Renderer::DrawRectangle(pos.x, pos.y, GetWidth(), GetHeight(), 0.0);
//    }
    if (bShowTooltip && !DialogSystemManager::IsDialogActive()) {
        const auto& pos = GetPosition();
        Renderer::DrawSprite(tooltipTexture, pos.x, pos.y - tooltipHeight - 16.0f, tooltipWidth, tooltipHeight);
    }
}

World::EInteractable* World::EPlayer::TryInteract() const {

    const auto& interactedEntities = MapManager::GetInteractedEntities();
    for (const auto& interactedEntityData : interactedEntities) {
        auto entity = MapManager::GetEntitiesContainer().FindEntity(interactedEntityData);
        if (entity) {
            return dynamic_cast<World::EInteractable*>(entity);
        }
    }

//    std::vector<World::EInteractable*> interactables;
//    MapManager::GetEntitiesContainer().FindEntities(interactables);
//    for (const auto& interactable : interactables) {
//        const auto interactablePosition = interactable->GetPosition();
//        const auto interactableWidthWithOffset = interactable->GetWidth() + 20.0f; // 20.0f то оффсет велью вывереный на глаз
//        const auto playerPosition = GetPosition();
//        const auto distance = glm::distance(interactablePosition, playerPosition);
//        if (distance < interactableWidthWithOffset) {
//            return interactable;
//        }
//    }
    return nullptr;
}
