#pragma once

#include <Renderer/Renderer.h>
#include <string>

namespace DialogSystemManager {
    void Initialize();
    void LoadAllDialogs(const std::string& directory);
    // characterId — ключ персонажа (например "spaghetti"), dialogId — ключ диалога (например "dialog-1")
    void StartDialog(const std::string& characterId, const std::string& dialogId);
    void EndDialog();
    bool IsDialogActive();
    void UpdateDialog();
    void RenderDialog();
}