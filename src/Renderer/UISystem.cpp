#include "UISystem.h"
#include "Renderer.h"
#include <Utils/Singleton.h>
#include <Game/GameplayLogic.h>
#include <ProgressSystem/ProgressSystem.h>
#include <ProgressSystem/InventarySaveData.h>

namespace {
    constexpr float INVENTORY_ICON_SIZE = 40.f;
    constexpr float INVENTORY_ICON_GAP = 2.f;
    constexpr float INVENTORY_MARGIN = 4.f;
}

class UISystemManager : public Singleton<UISystemManager> {
public:
    void Initialize() {
        fileSystemIconTextureId = make_nnTex("ui_filesystem");
        messageIconTextureId = make_nnTex("ui_message");
    }

    void Render() {
        // Инвентарь: иконки предметов в левом верхнем углу, 40x40, друг за другом

        // Отрисовка иконки файловой системы после её открытия в туториале
        if (GameplayLogic::GetCurrentGameActId() != "intro") {
            if (ProgressSystemManager::Player().fileSystemIconVisible) {
                Renderer::DrawSpriteScreen(fileSystemIconTextureId, 738, 2, 60, 66);
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

private:
    Renderer::TextureId fileSystemIconTextureId;
    Renderer::TextureId messageIconTextureId;
};

namespace UISystem {
    void Initialize() {
        UISystemManager::instance().Initialize();
    }

    void Render() {
        UISystemManager::instance().Render();
    }
}