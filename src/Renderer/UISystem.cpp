#include "UISystem.h"
#include "Renderer.h"
#include <Utils/Singleton.h>
#include <Game/GameplayLogic.h>
#include <ProgressSystem/ProgressSystem.h>

class UISystemManager : public Singleton<UISystemManager> {
public:
    void Initialize() {
        fileSystemIconTextureId = make_nnTex("ui_filesystem");
    }

    void Render() {
        // Отрисовка иконки файловой системы после её открытия в туториале
        if (GameplayLogic::GetCurrentGameActId() != "Intro") {
            if (ProgressSystemManager::Player().fileSystemIconVisible) {
                Renderer::DrawSpriteScreen(fileSystemIconTextureId, 738, 2, 60, 66);
            }
        }
    }

private:
    Renderer::TextureId fileSystemIconTextureId;
};

namespace UISystem {
    void Initialize() {
        UISystemManager::instance().Initialize();
    }

    void Render() {
        UISystemManager::instance().Render();
    }
}