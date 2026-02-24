#pragma once

namespace GameActs {

class GameAct {
public:
    virtual ~GameAct() = default;

    virtual void Initialize() {}
    // Апдейт для простых движений обьектов на карте
    virtual void Update(float /*deltaTime*/) {}
};

} // namespace GameActs