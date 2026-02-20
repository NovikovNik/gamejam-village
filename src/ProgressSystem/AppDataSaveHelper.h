#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <Game/GameFeatures.h>
#include <libs/json/single_include/nlohmann/json.hpp>
#include <libs/base64/base64.hpp>

#if defined(_WIN32)
    #include <windows.h>
    #include <shlobj.h>
#endif

namespace AppDataSaveHelper {

inline std::filesystem::path GetBaseAppDataPath() {
#if defined(_WIN32)
    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path) != S_OK) {
        return {};
    }
    std::filesystem::path result(path);
    CoTaskMemFree(path);
    return result;

#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home)
            / "Library"
            / "Application Support";
    }
    return {};

#else // Linux / Unix
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(xdg);
    }
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home)
            / ".local"
            / "share";
    }
    return {};
#endif
}

inline std::filesystem::path GetGameSavePath() {
#if ENABLE_CHEATS
    const std::filesystem::path saveFileName = "gamesave-debug.json";
#else
    const std::filesystem::path saveFileName = "gamesave.json";
#endif
    return GetBaseAppDataPath()
        / GameFeatures::gameTitle
        / saveFileName;
}

inline bool EnsureGameSaveDirectoryExists() {
    auto dir = GetGameSavePath().parent_path();
    return std::filesystem::exists(dir) ||
           std::filesystem::create_directories(dir);
}

inline bool GameSaveExists() {
    return std::filesystem::exists(GetGameSavePath());
}

inline bool SaveGameJson(const nlohmann::json& data) {
    if (!EnsureGameSaveDirectoryExists()) {
        return false;
    }

    const auto path = GetGameSavePath();
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

#if ENABLE_CHEATS
    file << data.dump(4);
#else
    file << base64::to_base64(data.dump(4));
#endif
    return static_cast<bool>(file);
}

inline bool LoadGameJson(nlohmann::json& outData) {
    const auto path = GetGameSavePath();
    if (!std::filesystem::exists(path)) {
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try {
#if ENABLE_CHEATS
        file >> outData;
#else
        std::string base64Data;
        file >> base64Data;
        outData = nlohmann::json::parse(base64::from_base64(base64Data));
#endif
    } catch (const nlohmann::json::parse_error&) {
        return false;
    } catch (const nlohmann::json::type_error&) {
        return false;
    }

    return true;
}

} // namespace AppDataSaveHelper