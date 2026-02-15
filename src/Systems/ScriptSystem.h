#pragma once

#include "../ECS/ECS.h"
#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ProjectTileEmitterComponent.h"
#include "sol/sol.hpp"
#include <tuple>

// Declare some native functions for bind

std::tuple <double, double> GetEntityPosition(Entity& entity) {
    if (entity.HasComponent<TransformComponent>()) {
        const auto& transform = entity.GetComponent<TransformComponent>();
        return std::make_tuple(transform.position.x, transform.position.y);
    } else {
        Logger::Err("Trying to get the position of an entity without transform component");
        return std::make_tuple(0.0, 0.0);
    }
}

void SetEntityPosition(Entity& entity, double x, double y) {
    if (entity.HasComponent<TransformComponent>()) {
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.position.x = x;
        transform.position.y = y;
    } else {
        Logger::Err("Trying to set the position of an entity without transform component");
    }
}

std::tuple <double, double> GetEntityVelocity(Entity& entity) {
    if (entity.HasComponent<RigidBodyComponent>()) {
        const auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
        return std::make_tuple(rigidBody.velocity.x, rigidBody.velocity.y);
    } else {
        Logger::Err("Trying to get the velocity of an entity without transform component");
        return std::make_tuple(0.0, 0.0);
    }
}

void SetEntityVelocity(Entity& entity, double x, double y) {
    if (entity.HasComponent<RigidBodyComponent>()) {
        auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
        rigidBody.velocity.x = x;
        rigidBody.velocity.y = y;
    } else {
        Logger::Err("Trying to set the position of an entity without transform component");
    }
}

void SetEntityRotation(Entity& entity, double angle) {
    if (entity.HasComponent<TransformComponent>()) {
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.rotation = angle;
    } else {
        Logger::Err("Trying to set rotation to an object without TransformComponent");
    }
}

double GetEntityRotation(Entity& entity) {
    if (entity.HasComponent<TransformComponent>()) {
        auto& transform = entity.GetComponent<TransformComponent>();
        return transform.rotation;
    } else {
        Logger::Err("Trying to get rotation to an object without TransformComponent");
    }
}

void SetProjectTileEmitterSpeed(Entity& entity, double speed) {
    if (entity.HasComponent<ProjectTileEmitterComponent>()) {
        auto& projectileEmitter = entity.GetComponent<ProjectTileEmitterComponent>();
        projectileEmitter.speed = speed;
    } else {
        Logger::Err("Trying to set project tile emitter speed to an object without projectileEmitter component");
    }
}

class ScriptSystem: public System {
    public:
    ScriptSystem() {
        RequireComponent<ScriptComponent>();
    }

    void CreateLuaBindings(sol::state& lua) {
        // Create the "eneity" usertype
        lua.new_usertype<Entity>("entity",
            "get_id", &Entity::GetId,
            "destroy", &Entity::Kill,
            "has_tag", &Entity::HasTag,
            "belongs_to_group", &Entity::BelongsToGroup
        );

        // Create all the bindings
        lua.set_function("set_position", SetEntityPosition);
        lua.set_function("get_position", GetEntityPosition);
        lua.set_function("get_rotation", GetEntityRotation);
        lua.set_function("set_angle", SetEntityRotation);
        lua.set_function("set_velocity", SetEntityVelocity);
        lua.set_function("get_velocity", GetEntityVelocity);
        lua.set_function("set_fire_speed", SetProjectTileEmitterSpeed);
    }

    void Update(float deltaTime, int elapsedTime) {
        for (auto& entity: GetSystemEntities()) {
            const auto& script = entity.GetComponent<ScriptComponent>();
            script.function(entity, deltaTime, elapsedTime);
        }
    }
};
