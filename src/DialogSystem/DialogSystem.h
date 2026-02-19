#pragma once

#include <Renderer/Renderer.h>
#include <string>
#include <array>
#include <cstdint>

/** Одна строка диалога: имя говорящего, текст, размер шрифта, цвет, выравнивание. */
struct DialogLine {
    std::string name;
    std::string text;
    int size = 20;
    std::array<std::uint8_t, 4> color = {255, 255, 255, 255};
    std::string align = "left";  // с какой стороны отрисовывать бокс имени
};

struct DialogData {
    std::vector<DialogLine> lines;
    int textSize = 20;  // дефолт для строк без своего size
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