#include "LocationNameOverlay.h"
#include "Renderer.h"
#include <Game/GameplayLogic.h>
#include <EventBus/EventBus.h>
#include <Events/LocationChangedEvent.h>
#include <string>

namespace {

enum class Phase { Idle, FadeIn, Hold, FadeOut };

constexpr float FADE_IN_DURATION = 0.5f;
constexpr float HOLD_DURATION = 2.0f;
constexpr float FADE_OUT_DURATION = 1.0f;
constexpr float TOP_MARGIN = 48.0f; // выверено на глаз
constexpr int FONT_SIZE = 28;

const Renderer::TextId FONT_ID = Renderer::TextId(make_nnTex("charriot"));

struct State {
    std::string locationName;
    float timer = 0.0f;
    Phase phase = Phase::Idle;
    float alpha = 0.0f;
};

State g_state;
Events::Handler g_onLocationChanged;

}

namespace LocationNameOverlay {

void Initialize() {
    g_onLocationChanged = EventBus::instance().SubscribeToEvent<LocationChangedEvent>(
        [] (LocationChangedEvent& e) {
            g_state.locationName = e.locationName;
            g_state.timer = 0.0f;
            g_state.phase = Phase::FadeIn;
            g_state.alpha = 0.0f;
        });
}

void Destroy() {
    g_onLocationChanged.Destroy();
}

void Update(float deltaTime) {
    if (GameplayLogic::GetCurrentGameActId() == GameActIds::Intro) {
        return;
    }
    if (g_state.phase == Phase::Idle) {
        return;
    }

    g_state.timer += deltaTime;

    switch (g_state.phase) {
    case Phase::FadeIn: {
        g_state.alpha = g_state.timer / FADE_IN_DURATION;
        if (g_state.alpha >= 1.0f) {
            g_state.alpha = 1.0f;
            g_state.timer = 0.0f;
            g_state.phase = Phase::Hold;
        }
        break;
    }
    case Phase::Hold:
        if (g_state.timer >= HOLD_DURATION) {
            g_state.timer = 0.0f;
            g_state.phase = Phase::FadeOut;
        }
        break;
    case Phase::FadeOut: {
        g_state.alpha = 1.0f - (g_state.timer / FADE_OUT_DURATION);
        if (g_state.alpha <= 0.0f) {
            g_state.alpha = 0.0f;
            g_state.locationName.clear();
            g_state.phase = Phase::Idle;
        }
        break;
    }
    default:
        break;
    }
}

void Render() {
    if (GameplayLogic::GetCurrentGameActId() == GameActIds::Intro) {
        return;
    }
    if (g_state.locationName.empty()) {
        return;
    }
    int screenW = 0, screenH = 0;
    Renderer::GetRenderOutputSize(&screenW, &screenH);

    int textW = 0, textH = 0;
    if (!Renderer::MeasureText(FONT_ID, g_state.locationName, FONT_SIZE, &textW, &textH)) {
        return;
    }

    const float x = (screenW - textW) * 0.5f;
    const float y = TOP_MARGIN;

    Renderer::DrawTextScreen(FONT_ID, g_state.locationName, x, y, FONT_SIZE, nullptr, 0, g_state.alpha);
}

}
