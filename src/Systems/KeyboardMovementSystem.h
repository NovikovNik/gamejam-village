#pragma once
#include "../ECS/ECS.h"
#include "../Game/GameFeatures.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/KeyBoardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "SDL_keycode.h"
#include <string>

class KeyBoardMovementSystem: public System {
    public:
        KeyBoardMovementSystem() {
            RequireComponent<KeyBoardControlledComponent>();
            RequireComponent<SpriteComponent>();
            RequireComponent<RigidBodyComponent>();
        }

        void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
            eventBus->SubscribeToEvent<KeyPressedEvent, KeyBoardMovementSystem>(this, &KeyBoardMovementSystem::onKeyPressed);
        }

        void onKeyPressed(KeyPressedEvent& event) {
            std::string keyCode = std::to_string(event.key);
            std::string KeySymbol(1, event.key);

            for (auto entity: GetSystemEntities()) {
                const auto& keyboardControl = entity.GetComponent<KeyBoardControlledComponent>();
                auto& sprite = entity.GetComponent<SpriteComponent>();
                auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

                Logger::Log("KEY: [" + keyCode + "] [ " + KeySymbol + "]");

                switch(event.key) {
                    case SDLK_UP:
                        if (!GameFeatures::isDebug) {
                            rigidBody.velocity = keyboardControl.upVelocity;
                            sprite.srcRect.y = sprite.height * 0;
                            sprite.direction = Direction::UP;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (!GameFeatures::isDebug) {
                            rigidBody.velocity = keyboardControl.rightVelocity;
                            sprite.srcRect.y = sprite.height * 1;
                            sprite.direction = Direction::RIGHT;
                        }
                        break;
                    case SDLK_DOWN:
                        if (!GameFeatures::isDebug) {
                            rigidBody.velocity = keyboardControl.downVelocity;
                            sprite.srcRect.y = sprite.height * 2;
                            sprite.direction = Direction::DOWN;
                        }
                        break;
                    case SDLK_LEFT:
                        if (!GameFeatures::isDebug) {
                            rigidBody.velocity = keyboardControl.leftVelocity;
                            sprite.srcRect.y = sprite.height * 3;
                            sprite.direction = Direction::LEFT;
                        }
                        break;
                }
            }
        }

        void Update() {
        }
};
