#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <Game/GameFeatures.h>
#include <libs/json/single_include/nlohmann/json.hpp>

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
    return GetBaseAppDataPath()
        / GameFeatures::gameTitle
        / "gamesave.json";
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

    file << data.dump(4);
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
        file >> outData;
    } catch (const nlohmann::json::parse_error&) {
        return false;
    }

    return true;
}

} // namespace AppDataSaveHelper