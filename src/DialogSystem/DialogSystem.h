#pragma once

#include <Renderer/Renderer.h>
#include <string>

struct DialogData {
    std::vector<std::string> lines;
    int textSize = 20;
};

using DialogsMap = std::map<std::string, std::map<std::string, DialogData>>;

namespace DialogSystemManager {
    void Initialize();
    void Destroy();
    void LoadAllDialogs(const std::string& directory);
    // characterId — ключ персонажа (например "spaghetti"), dialogId — ключ диалога (например "dialog-1")
    void StartDialog(const std::string& characterId, const std::string& dialogId);
    void EndDialog();
    bool IsDialogActive();
    void UpdateDialog();
    void RenderDialog();
    [[nodiscard]] const DialogsMap& GetDialogs();
}