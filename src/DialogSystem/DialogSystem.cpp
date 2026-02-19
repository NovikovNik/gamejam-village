#include "DialogSystem.h"
#include <Logger/Logger.h>
#include <Utils/Singleton.h>
#include <FileSystem/FileSystem.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <EventBus/EventBus.h>
#include <Events/NextDialogLineEvent.h>
#include <Events/ForceDialogStartEvent.h>
#include <Events/DialogEndedEvent.h>
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
        dialogBackgroundTexture = make_nnTex("dialog_ui_texture");
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
                    json["name"].get<std::string>(),
                    json["description"].get<std::string>()
                };

                for (auto& [dialogId, dialogJson] : json["dialogs"].items()) {
                    if (dialogId == "textSize") continue; // пропускаем глобальный textSize если ошибочно в dialogs
                    DialogData data;
                    if (dialogJson.is_array()) {
                        data.lines = dialogJson.get<std::vector<std::string>>();
                    } else if (dialogJson.is_object()) {
                        data.lines = dialogJson["lines"].get<std::vector<std::string>>();
                        if (dialogJson.contains("textSize")) {
                            data.textSize = dialogJson["textSize"].get<int>();
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
        currentDialogText = "";
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

        const DialogData& data = itDialog->second;
        dialogActive = true;
        currentCharacterId = characterId;
        currentDialogId = dialogId;
        currentLines = data.lines;
        currentDialogTextSize = data.textSize;
        currentDialogIndex = 0;
        currentDialogText = currentLines.empty() ? "" : currentLines[0];

        Logger::Log(std::format("[DialogSystem] Dialog: {}:{} started", characterId, dialogId));
    }

    void EndDialog() {
        dialogActive = false;
        EventBus::instance().EmitEvent<DialogEndedEvent>(currentCharacterId, currentDialogId);
        currentCharacterId = "";
        currentDialogId = "";
        currentDialogIndex = 0;
        currentLines.clear();
        currentDialogText = "";
        currentDialogTextSize = 20;
    }

    // Чтобы узнавать статус диалоговой системы
    bool IsDialogActive() const {
        return dialogActive;
    }

    [[maybe_unused]] void UpdateDialog() {}

    void RenderDialog() {
        if (!dialogActive) return;
        Renderer::DrawSpriteScreen(dialogBackgroundTexture, 2, 380, 793, 216);
        // Render character name
        Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), characterMeta[currentCharacterId].name, 15, 404, 22, nullptr, 300);
        // Render dialog text
        Renderer::DrawTextScreen(Renderer::TextId(make_nnTex("charriot")), currentDialogText, 15, 465, currentDialogTextSize, nullptr, 790);
    }

    void OnNextDialogLineEvent(NextDialogLineEvent&) {
        if (!dialogActive) return;

        currentDialogIndex++;
        if (currentDialogIndex < static_cast<int>(currentLines.size())) {
            currentDialogText = currentLines[currentDialogIndex];
        } else {
            EndDialog();
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
    std::string currentCharacterId;
    std::string currentDialogId;
    int currentDialogIndex = 0;
    std::vector<std::string> currentLines;
    std::string currentDialogText;
    int currentDialogTextSize = 20;

    Renderer::TextureId dialogBackgroundTexture;

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
