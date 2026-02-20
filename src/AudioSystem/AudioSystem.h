#pragma once

#include <string>

namespace AudioSystem {

    using SoundId = std::string;

    void Initialize();
    void Destroy();
    /// Call once per frame: cleans up finished SFX streams and refills looping music.
    void Update();

    /// Loads all .wav files from the given directory. The stem of each filename
    /// becomes the SoundId (e.g. "assets/audio/jump.wav" → id "jump").
    void LoadAllSounds(const std::string& directory);

    /// Play a one-shot sound effect. volume is a [0,1] scale applied on top of sfx volume.
    void PlaySound(const SoundId& id, float volume = 1.0f);

    /// Start looping background music. Replaces any currently playing track.
    void PlayMusic(const SoundId& id);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying();

    /// All volumes are in [0, 1].
    /// GetMasterVolume always returns the true (pre-mute) level — safe to read for UI/save.
    void SetMasterVolume(float volume);
    void SetSfxVolume(float volume);
    void SetMusicVolume(float volume);

    float GetMasterVolume();
    float GetSfxVolume();
    float GetMusicVolume();

    /// Mute silences output without changing the saved master volume.
    void SetMuted(bool muted);
    bool IsMuted();

}
