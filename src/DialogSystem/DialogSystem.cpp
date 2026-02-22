#include "DialogSystem.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <FileSystem/FileSystem.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <EventBus/EventBus.h>
#include <Events/NextDialogLineEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/DialogEndedEvent.h>
#include <Events/PlaySoundEvent.h>
#include <SDL3/SDL.h>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <format>
#include <vector>

struct CharacterMeta {
    std::string name;
    std::string description;
};

class DialogSystem: public Singleton<DialogSystem> {
public:

    void Initialize() {
        dialogBackgroundNameTexture = make_nnTex("dialog_ui_speeker");
        dialogBackgroundTexture = make_nnTex("dialog_ui_texture");
        signBackgroundTexture = make_nnTex("wooden_board_ui");
        signPlateTexture = make_nnTex("name_sign_ui");
        onNextDialogLineEvent = EventBus::instance().SubscribeToEvent<NextDialogLineEvent>(this, &DialogSystem::OnNextDialogLineEvent);
        onForceDialogStartEvent = EventBus::instance().SubscribeToEvent<ForceDialogStartEvent>(this, &DialogSystem::OnForceDialogStartEvent);
    }

    void Destroy() {
        onNextDialogLineEvent.Destroy();
        onForceDialogStartEvent.Destroy();
    }

    void LoadAllDialogs(const std::string& directory) {

        // std::filesystem::path directory = FileSystemManager::GetExecutableDir() / "assets/dialogs";
        if (!std::filesystem::exists(directory)) {
            Logger::Err("Dialogs path does not exist: " + directory);
            return;
        }
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                nlohmann::json json = LoadDialogData(entry.path().string());
                if (json.is_null()) continue;

                const std::string characterId = json["id"].get<std::string>();
                characterMeta[characterId] = {
                    json.value("name", characterId),
                    json.value("description", std::string())
                };

                for (auto& [dialogId, dialogJson] : json["dialogs"].items()) {
                    if (dialogId == "textSize") continue; // пропускаем глобальный textSize если ошибочно в dialogs
                    DialogData data;
                    if (dialogJson.is_array()) {
                        for (const auto& item : dialogJson) {
                            if (item.is_string()) {
                                data.lines.push_back({"", item.get<std::string>(), data.textSize, {255,255,255,255}, "left"});
                            } else if (item.is_object()) {
                                data.lines.push_back(ParseDialogLine(item, data.textSize));
                            }
                        }
                    } else if (dialogJson.is_object() && dialogJson.contains("lines")) {
                        if (dialogJson.contains("textSize")) {
                            data.textSize = dialogJson["textSize"].get<int>();
                        }
                        for (const auto& item : dialogJson["lines"]) {
                            if (item.is_string()) {
                                data.lines.push_back({"", item.get<std::string>(), data.textSize, {255,255,255,255}, "left"});
                            } else if (item.is_object()) {
                                data.lines.push_back(ParseDialogLine(item, data.textSize));
                            }
                        }
                    }
                    dialogs[characterId][dialogId] = std::move(data);
                }

                Logger::Log(std::format("[DialogSystem] Dialog: {} loaded ({} dialogs)", entry.path().stem().string(), json["dialogs"].size()));
            }
        }

        dialogActive = false;
        currentCharacterId = "";
        currentDialogId = "";
        currentDialogIndex = 0;
        currentLines.clear();
        Logger::Log("[DialogSystem] Initialized");
    }

    void StartDialog(const std::string& characterId, const std::string& dialogId) {
        if (dialogActive) {
            Logger::Log("Dialog already active");
            return;
        }
        auto itChar = dialogs.find(characterId);
        if (itChar == dialogs.end()) {
            Logger::Err(std::format("Character '{}' not found", characterId));
            return;
        }
        auto itDialog = itChar->second.find(dialogId);
        if (itDialog == itChar->second.end()) {
            Logger::Err(std::format("Dialog '{}' not found for character '{}'", dialogId, characterId));
            return;
        }

        EventBus::instance().EmitEvent<PlaySoundEvent>("dialog_start");
        const DialogData& data = itDialog->second;
        dialogActive = true;
        currentCharacterId = characterId;
        currentDialogId = dialogId;
        currentLines = data.lines;
        currentDialogTextSize = data.textSize;
        currentDialogIndex = 0;

        Logger::Log(std::format("[DialogSystem] Dialog: {}:{} started", characterId, dialogId));
    }

    void OpenSign(const std::vector<std::string>& rows) {
        signRows = rows;
        signActive = true;
    }

    void CloseSign() {
        signRows.clear();
        signActive = false;
    }

    void EndDialog() {
        dialogActive = false;
        EventBus::instance().EmitEvent<DialogEndedEvent>(currentCharacterId, currentDialogId);
        currentCharacterId = "";
        currentDialogId = "";
        currentDialogIndex = 0;
        currentLines.clear();
        currentDialogTextSize = 20;
    }

    // Чтобы узнавать статус диалоговой системы
    bool IsDialogActive() const {
        return dialogActive || signActive;
    }

    [[maybe_unused]] void UpdateDialog() {}

    void RenderDialog() {
        if (signActive) {
            Renderer::DrawSpriteScreen(signBackgroundTexture, 55, 55, 387*1.8f, 270*1.8f);
            for (int32_t rowId = 0; const auto& row : signRows) {
                Renderer::DrawSpriteScreen(signPlateTexture, 55 + 348 - 185, 80 + rowId * 55.5f, 206*1.8f, 29*1.8f);
                static constexpr SDL_Color black{ 0, 0, 0, 255 };
                Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), row, 110 + 348 - 185, 80 + rowId * 55.5f + 10, 32, &black, 206*1.8f);
                rowId++;
            }
            return;
        }

        if (!dialogActive || currentDialogIndex >= static_cast<int>(currentLines.size())) {
            return;
        }

        const DialogLine& line = currentLines[currentDialogIndex];
        SDL_Color textColor = {line.color[0], line.color[1], line.color[2], line.color[3]};

        Renderer::DrawSpriteScreen(dialogBackgroundTexture, 3, 448, 793, 147);
        Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), line.text, 12, 465, line.size, &textColor, 790);

        // Имя говорящего: из текущей строки или из меты персонажа
        std::string speakerName = line.name.empty() ? characterMeta[currentCharacterId].name : line.name;
        // Рендер имени говорящего и бокса под него в нужном месте (слева или справа)
        if (line.align == "left") {
            Renderer::DrawSpriteScreen(dialogBackgroundNameTexture, 3, 385, 308, 59);
            Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), speakerName, 12, 404, 22, nullptr, 300);
        } else if (line.align == "right") {
            Renderer::DrawSpriteScreen(dialogBackgroundNameTexture, 489, 385, 308, 59);
            Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), speakerName, 540, 404, 22, nullptr, 300);
        }
    }

    void OnNextDialogLineEvent(NextDialogLineEvent&) {
        if (signActive) {
            signActive = false;
            return;
        }
        if (!dialogActive) {
            return;
        }

        currentDialogIndex++;
        if (currentDialogIndex >= static_cast<int>(currentLines.size())) {
            EndDialog();
        } else {
            EventBus::instance().EmitEvent<PlaySoundEvent>("dialog_next");
        }
    }

    // Возможность форсированно запустить нужный диалог без взаимодействия с NPC например
    // Гипотетический рассказчик или обучение?
    void OnForceDialogStartEvent(ForceDialogStartEvent& event) {
        StartDialog(event.characterId, event.dialogId);
    }

    [[nodiscard]] const CharacterMeta* GetCharacterMeta(const std::string& characterId) const {
        auto it = characterMeta.find(characterId);
        return it != characterMeta.end() ? &it->second : nullptr;
    }

    [[nodiscard]] const DialogsMap& GetDialogs() {
        return dialogs;
    }

private:
    static DialogLine ParseDialogLine(const nlohmann::json& item, int defaultSize) {
        DialogLine line;
        line.name = item.value("name", std::string());
        line.text = item.value("text", std::string());
        line.size = item.value("size", defaultSize);
        line.align = item.value("align", std::string("left"));
        if (item.contains("color") && item["color"].is_array() && item["color"].size() >= 4) {
            line.color[0] = static_cast<std::uint8_t>(item["color"][0].get<int>());
            line.color[1] = static_cast<std::uint8_t>(item["color"][1].get<int>());
            line.color[2] = static_cast<std::uint8_t>(item["color"][2].get<int>());
            line.color[3] = static_cast<std::uint8_t>(item["color"][3].get<int>());
        }
        return line;
    }

    nlohmann::json LoadDialogData(const std::string& filepath) const {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Err("Failed to open dialog file: " + filepath);
            return {};
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return nlohmann::json::parse(buffer.str());
    }

    bool dialogActive = false;
    bool signActive = false;
    std::vector<std::string> signRows;

    std::string currentCharacterId;
    std::string currentDialogId;
    int currentDialogIndex = 0;
    std::vector<DialogLine> currentLines;
    int currentDialogTextSize = 20;

    Renderer::TextureId dialogBackgroundNameTexture;
    Renderer::TextureId dialogBackgroundTexture;

    Renderer::TextureId signBackgroundTexture;
    Renderer::TextureId signPlateTexture;

    DialogsMap dialogs;
    std::map<std::string, CharacterMeta> characterMeta;

    Events::Handler onNextDialogLineEvent;
    Events::Handler onForceDialogStartEvent;
};

void DialogSystemManager::Initialize() {
    DialogSystem::instance().Initialize();
}

void DialogSystemManager::LoadAllDialogs(const std::string& directory) {
    DialogSystem::instance().LoadAllDialogs(directory);
}

void DialogSystemManager::StartDialog(const std::string& characterId, const std::string& dialogId) {
    DialogSystem::instance().StartDialog(characterId, dialogId);
}

void DialogSystemManager::EndDialog() {
    DialogSystem::instance().EndDialog();
}

void DialogSystemManager::OpenSign(const std::vector<std::string>& rows) {
    DialogSystem::instance().OpenSign(rows);
}

void DialogSystemManager::CloseSign() {
    DialogSystem::instance().CloseSign();
}

bool DialogSystemManager::IsDialogActive() {
    return DialogSystem::instance().IsDialogActive();
}

void DialogSystemManager::UpdateDialog() {
    DialogSystem::instance().UpdateDialog();
}

void DialogSystemManager::RenderDialog() {
    DialogSystem::instance().RenderDialog();
}

// Читовая функция
[[nodiscard]] const DialogsMap& DialogSystemManager::GetDialogs() {
    return DialogSystem::instance().GetDialogs();
}

void DialogSystemManager::Destroy() {
    DialogSystem::instance().Destroy();
}
