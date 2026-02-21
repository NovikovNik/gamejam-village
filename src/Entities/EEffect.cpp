#include "EEffect.h"

World::Effect::Effect(const Renderer::AnimationHandle& animation) : animation(animation) {
}

bool World::Effect::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    if (isFinished) {
        Destroy();
    }

    return isValid;
}

void World::Effect::Render(float deltaTime) {
    if (*animation.textureId == 0 || isFinished) {
        return;
    }

    isFinished = Renderer::RenderAnimation(animation, deltaTime, positionX, positionY, width, height);
}
