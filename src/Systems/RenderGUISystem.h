#pragma once

#include "../ECS/ECS.h"
#include <SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <string>

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectTileEmitterComponent.h"
#include "../Components/HealthBarComponent.h"
#include "../Components/HealthComponent.h"


class RenderGUISystem: public System {
    private:
        const char* enemyList[2] = { "truck-image", "tank-image" };
    public:
        RenderGUISystem() = default;

        void Update(SDL_Renderer* renderer, Registry& registry, SDL_Rect& camera) {
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            if (ImGui::Begin("Entity Spawner")) {
                // TODO: Input for X pos
                static int enemyX, enemyY = 0;
                static int velocityX, velocityY = 0;
                static int enemyScale = 1;
                static float enemyRad = 0;
                static int emenyHP = 100;
                static float bulletSpeed = 100;
                static float bulletDamage = 10;
                static float bulletTTL = 5;
                static float emitterFrequency = 1.0;
                static int item_current = 1;

                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::InputInt("Entity X: ", &enemyX);
                    ImGui::InputInt("Entity Y: ", &enemyY);
                    ImGui::SliderAngle("Entity Angle (deg):", &enemyRad);
                    ImGui::InputInt("Entity Scale: ", &enemyScale);
                    ImGui::Combo("listbox", &item_current, enemyList, IM_ARRAYSIZE(enemyList), 4);
                }
                ImGui::Spacing();

                if (ImGui::CollapsingHeader("Regid Body", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::InputInt("Velocity X: ", &velocityX);
                    ImGui::InputInt("Velocity Y: ", &velocityY);
                }

                if (ImGui::CollapsingHeader("Healt System", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderInt("%", &emenyHP, 0, 100);
                }
                ImGui::Spacing();

                if (ImGui::CollapsingHeader("Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::InputFloat("Freaquency (sec): ", &emitterFrequency);
                    ImGui::InputFloat("Speed (px/sec): ", &bulletSpeed);
                    ImGui::InputFloat("Damage (%): ", &bulletDamage);
                    ImGui::InputFloat("TTL (sec): ", &bulletTTL);
                }
                ImGui::Spacing();

                if (ImGui::Button("Create entity")) {
                    Entity enemy = registry.CreateEntity();
                    Logger::Warn("Enemy: RAD: " + std::to_string(enemyRad));
                    enemy.AddComponent<TransformComponent>(glm::vec2(enemyX, enemyY), glm::vec2(enemyScale, enemyScale), enemyRad * (180 / M_PI));
                    enemy.AddComponent<RigidBodyComponent>(glm::vec2(velocityX, velocityY));
                    enemy.AddComponent<SpriteComponent>(enemyList[item_current], 1, 32, 32, false);
                    enemy.AddComponent<BoxColliderComponent>(32, 32);
                    enemy.AddComponent<ProjectTileEmitterComponent>(bulletSpeed, emitterFrequency * 1000, bulletTTL * 1000, bulletDamage, false);
                    enemy.AddComponent<HealthComponent>(emenyHP);
                    enemy.AddComponent<HealthBarComponent>();
                    enemy.Group("enemies");
                }
                // TODO: Input for Y pos
                // TODO: A button to create a new entity
            }
            ImGui::End();

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiChildFlags_AlwaysAutoResize;
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
            ImGui::SetNextWindowBgAlpha(0.9f);
            if (ImGui::Begin("Map coordinates", NULL, windowFlags)) {
                ImGui::Text(
                    "Map Coordinates: (x=%1.f, y=%1.f)",
                    ImGui::GetIO().MousePos.x + camera.x,
                    ImGui::GetIO().MousePos.y + camera.y
                );
                ImGui::Text(
                    "Map Size: (x=%1.d, y=%1.d)",
                    Game::mapWidth,
                    Game::mapHeight
                );
                ImGui::Text(
                    "Camera Pos: (x=%1.d, y=%1.d)",
                    camera.x,
                    camera.y
                );
                std::optional<Entity> entity = registry.GetEntityByTag("Player");
                if (entity.has_value()) {
                    const auto& transform = entity.value().GetComponent<TransformComponent>();
                    ImGui::Text(
                        "Player Pos: (x=%1.f, y=%1.f)",
                        transform.position.x,
                        transform.position.y
                    );
                }
            }
            ImGui::End();

            ImGui::Render();
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        }
};
