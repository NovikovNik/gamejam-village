#pragma once
#include "BasicSaveData.h"

struct AudioSaveData : public BasicSaveData {
    float masterVolume = 1.0f;
    float musicVolume  = 1.0f;
    float sfxVolume    = 0.008f;
    bool  muted        = false;

    int GetVersion() const override { return 1; }

    void ResetToDefaults() override {
        masterVolume = 1.0f;
        musicVolume  = 1.0f;
        sfxVolume    = 0.008f;
        muted        = false;
    }

    void ToJson(nlohmann::json& j) const override {
        j["version"] = GetVersion();
        j["master"]  = masterVolume;
        j["music"]   = musicVolume;
        j["sfx"]     = sfxVolume;
        j["muted"]   = muted;
    }

    void FromJson(const nlohmann::json& j) override {
        if (!j.is_object()) { ResetToDefaults(); return; }
        masterVolume = j.value("master", 1.0f);
        musicVolume  = j.value("music",  1.0f);
        sfxVolume    = j.value("sfx",    0.008f);
        muted        = j.value("muted",  false);
        if (!Validate()) ResetToDefaults();
    }

    [[maybe_unused]] bool Validate() const override {
        return masterVolume >= 0.f && masterVolume <= 1.f
            && musicVolume  >= 0.f && musicVolume  <= 1.f
            && sfxVolume    >= 0.f && sfxVolume    <= 1.f;
    }
};
