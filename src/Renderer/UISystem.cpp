#include "UISystem.h"
#include "Renderer.h"
#include <Utils/Singleton.h>
#include <Game/GameplayLogic.h>
#include <ProgressSystem/ProgressSystem.h>
#include <ProgressSystem/InventarySaveData.h>
#include <array>
#include <string>

namespace {
    constexpr float INVENTORY_ICON_SIZE = 40.f;
    constexpr float INVENTORY_ICON_GAP = 2.f;
    constexpr float INVENTORY_MARGIN = 4.f;

    // Credits
    enum class CreditsPhase { FadeIn, Hold, FadeOut, Delay };
    constexpr float CREDITS_FADE_DURATION = 1.0f;
    constexpr float CREDITS_HOLD_DURATION = 3.5f;
    constexpr float CREDITS_DELAY_BETWEEN = 0.8f;
    constexpr int CREDITS_FONT_SIZE = 26;

    constexpr std::array<const char*, 5> CREDITS_LINES = {
        "AAAB Made for the Brackeys Game Jam 2026.1",
        "Programming & Story: Nikolai Novikov, Ruslan Zhuchkov",
        "Art & Story: Arsenii Kulakov",
        "Special thanks to cheesybun for cover image",
        "Thank you!",
    };
}

class UISystemManager : public Singleton<UISystemManager> {
public:
    void Initialize() {
        fileSystemIconTextureId = make_nnTex("ui_filesystem");
        messageIconTextureId = make_nnTex("ui_message");

        focusHintAnimation = Renderer::AnimationHandle {
            .numOfFrames = 4,
            .maxElementsPerRow = 2,
            .frameSize = 66,
            .frameDelay = 0.1f,
            .textureId = make_nnTex("ui_filesystem_anim"),
        };
    }

    void Render(float deltaTime) {
        // Инвентарь: иконки предметов в левом верхнем углу, 40x40, друг за другом
        // Отрисовка иконки файловой системы после её открытия в туториале
        const bool gameEnded = ProgressSystemManager::Player().gameEnded;
        if (gameEnded) {
            UpdateAndRenderCredits(deltaTime);
        } else {
            creditsStarted = false;
            if (GameplayLogic::GetCurrentGameActId() != "intro") {
                if (ProgressSystemManager::Player().fileSystemIconVisible) {

                    if (focusHintTimeLeft > 0.f) {
                        focusHintTimeLeft -= deltaTime;
                        Renderer::RenderAnimationScreen(focusHintAnimation, deltaTime, 736, 2, 66, 66);
                    } else {
                        Renderer::DrawSpriteScreen(fileSystemIconTextureId, 736, 2, 66, 66);
                    }
                }

                // Отрисовка инвентаря
                int index = 0;
                for (const auto& itemId : ProgressSystemManager::Inventory().GetItems()) {
                    const auto textureId = make_nnTex("ui_" + itemId);
                    if (!Renderer::HasTexture(textureId)) {
                        continue;
                    }
                    const float x = INVENTORY_MARGIN + index * (INVENTORY_ICON_SIZE + INVENTORY_ICON_GAP);
                    const float y = INVENTORY_MARGIN;
                    Renderer::DrawSpriteScreen(textureId, x, y, INVENTORY_ICON_SIZE, INVENTORY_ICON_SIZE);
                    ++index;
                }
            }
        }
    }

    void UpdateAndRenderCredits(float deltaTime) {
        if (!creditsStarted) {
            creditsStarted = true;
            creditsSlideIndex = 0;
            creditsPhase = CreditsPhase::FadeIn;
            creditsTimer = 0.f;
            creditsAlpha = 0.f;
        }

        const std::string& line = CREDITS_LINES[creditsSlideIndex];
        creditsTimer += deltaTime;

        switch (creditsPhase) {
        case CreditsPhase::FadeIn: {
            creditsAlpha = creditsTimer / CREDITS_FADE_DURATION;
            if (creditsAlpha >= 1.f) {
                creditsAlpha = 1.f;
                creditsTimer = 0.f;
                creditsPhase = CreditsPhase::Hold;
            }
            break;
        }
        case CreditsPhase::Hold:
            if (creditsTimer >= CREDITS_HOLD_DURATION) {
                creditsTimer = 0.f;
                creditsPhase = CreditsPhase::FadeOut;
            }
            break;
        case CreditsPhase::FadeOut: {
            creditsAlpha = 1.f - (creditsTimer / CREDITS_FADE_DURATION);
            if (creditsAlpha <= 0.f) {
                creditsAlpha = 0.f;
                creditsTimer = 0.f;
                if (creditsSlideIndex + 1 >= static_cast<int>(CREDITS_LINES.size())) {
                    creditsSlideIndex = 0;
                    creditsPhase = CreditsPhase::FadeIn;
                } else {
                    creditsPhase = CreditsPhase::Delay;
                }
            }
            break;
        }
        case CreditsPhase::Delay:
            if (creditsTimer >= CREDITS_DELAY_BETWEEN) {
                creditsTimer = 0.f;
                ++creditsSlideIndex;
                creditsPhase = CreditsPhase::FadeIn;
            }
            break;
        }

        if (creditsPhase != CreditsPhase::Delay) {
            const auto fontId = Renderer::TextId(make_nnTex("charriot"));
            int screenW = 0, screenH = 0;
            Renderer::GetRenderOutputSize(&screenW, &screenH);
            int textW = 0, textH = 0;
            if (Renderer::MeasureText(fontId, line, CREDITS_FONT_SIZE, &textW, &textH)) {
                const float x = (screenW - textW) * 0.5f;
                const float y = (screenH - textH) * 0.5f;
                Renderer::DrawTextScreen(fontId, line, x, y, CREDITS_FONT_SIZE, nullptr, 0, creditsAlpha);
            }
        }
    }

    void FocusHint() {
        focusHintTimeLeft = 2.f;
    }

private:
    Renderer::TextureId fileSystemIconTextureId;
    Renderer::TextureId messageIconTextureId;

    Renderer::AnimationHandle focusHintAnimation;

    float focusHintTimeLeft = 0.f;

    bool creditsStarted = false;
    int creditsSlideIndex = 0;
    CreditsPhase creditsPhase = CreditsPhase::FadeIn;
    float creditsTimer = 0.f;
    float creditsAlpha = 0.f;
};

namespace UISystem {
    void Initialize() {
        UISystemManager::instance().Initialize();
    }

    void Render(float deltaTime) {
        UISystemManager::instance().Render(deltaTime);
    }

    void FocusHint() {
        UISystemManager::instance().FocusHint();
    }
}