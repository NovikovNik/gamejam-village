#include "AudioSystem.h"

#include <SDL3/SDL.h>
#include <Logger/Logger.h>
#include <EventBus/EventBus.h>
#include <Events/PlaySoundEvent.h>
#include <Events/PlayMusicEvent.h>

#include <FileSystem/FileSystem.h>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

// Single-file lib: include .c with HEADER_ONLY for declarations; implementation from libs/stb_vorbis/stb_vorbis.c in build
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"
#undef STB_VORBIS_HEADER_ONLY

namespace {

struct SoundData {
    SDL_AudioSpec spec{};
    std::vector<Uint8> buffer;
};

SDL_AudioDeviceID g_device = 0;

std::unordered_map<std::string, SoundData> g_sounds;

// Fire-and-forget SFX streams. Cleaned up in Update() when drained.
std::vector<SDL_AudioStream*> g_sfxStreams;

// Looping music
SDL_AudioStream* g_musicStream   = nullptr;
std::string      g_currentMusic;
bool             g_musicPlaying  = false;
bool             g_musicPaused   = false;

float g_masterVolume = 1.0f;
float g_sfxVolume    = 0.02f;
float g_musicVolume  = 1.0f;
bool  g_muted        = false;

Events::Handler g_onPlaySound;
Events::Handler g_onPlayMusic;

// ---------------------------------------------------------------------------

void CleanupFinishedSfxStreams() {
    auto it = g_sfxStreams.begin();
    while (it != g_sfxStreams.end()) {
        // SDL_GetAudioStreamQueued returns bytes still waiting to be played.
        // When it reaches 0 the stream has been fully consumed.
        if (SDL_GetAudioStreamQueued(*it) == 0) {
            SDL_DestroyAudioStream(*it); // auto-unbinds from device
            it = g_sfxStreams.erase(it);
        } else {
            ++it;
        }
    }
}

void RefillMusicStream() {
    if (!g_musicStream || !g_musicPlaying || g_musicPaused) return;

    auto it = g_sounds.find(g_currentMusic);
    if (it == g_sounds.end()) return;

    const SoundData& music = it->second;

    // Keep at least one full loop worth of data queued for seamless looping.
    if (SDL_GetAudioStreamQueued(g_musicStream) < static_cast<int>(music.buffer.size())) {
        SDL_PutAudioStreamData(g_musicStream,
                               music.buffer.data(),
                               static_cast<int>(music.buffer.size()));
    }
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

namespace AudioSystem {

void Initialize() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        Logger::Err("[AudioSystem] SDL_InitSubSystem(AUDIO) failed: " + std::string(SDL_GetError()));
        return;
    }

    g_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!g_device) {
        Logger::Err("[AudioSystem] SDL_OpenAudioDevice failed: " + std::string(SDL_GetError()));
        return;
    }

    SDL_SetAudioDeviceGain(g_device, g_masterVolume);
    Logger::Log("[AudioSystem] Initialized");

    g_onPlaySound = EventBus::instance().SubscribeToEvent<PlaySoundEvent>(
        [](PlaySoundEvent& e) { PlaySound(e.soundId, e.volume); });

    g_onPlayMusic = EventBus::instance().SubscribeToEvent<PlayMusicEvent>(
        [](PlayMusicEvent& e) { PlayMusic(e.musicId); });
}

void Destroy() {
    g_onPlaySound.Destroy();
    g_onPlayMusic.Destroy();

    // Destroy music stream first (auto-unbinds)
    if (g_musicStream) {
        SDL_DestroyAudioStream(g_musicStream);
        g_musicStream = nullptr;
    }

    // Destroy all live SFX streams
    for (auto* s : g_sfxStreams) {
        SDL_DestroyAudioStream(s);
    }
    g_sfxStreams.clear();

    if (g_device) {
        SDL_CloseAudioDevice(g_device);
        g_device = 0;
    }

    g_sounds.clear();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    Logger::Log("[AudioSystem] Destroyed");
}

void Update() {
    if (!g_device) return;
    CleanupFinishedSfxStreams();
    RefillMusicStream();
}

void LoadAllSounds(const std::string& directory) {
    namespace fs = std::filesystem;

#if !ENABLE_CHEATS
    std::string soundsDirectory = (FileSystemManager::GetExecutableDir() / "assets/audio").string();
#else
    std::string soundsDirectory = directory;
#endif

    if (!fs::exists(soundsDirectory)) {
        Logger::Err("[AudioSystem] Sound directory not found: " + directory);
        return;
    }

    int loaded = 0;
    for (const auto& entry : fs::recursive_directory_iterator(soundsDirectory)) {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        const bool isWav = (ext == ".wav");
        const bool isOgg = (ext == ".ogg");
        if (!isWav && !isOgg)
        {
            continue;
        }

        const std::string path = entry.path().string();
        const std::string id   = entry.path().stem().string();

        SoundData data;

        if (isWav) {
            Uint8* raw = nullptr;
            Uint32 len = 0;
            if (!SDL_LoadWAV(path.c_str(), &data.spec, &raw, &len)) {
                Logger::Err("[AudioSystem] Failed to load WAV '" + path + "': " + SDL_GetError());
                continue;
            }
            data.buffer.assign(raw, raw + len);
            SDL_free(raw);
        } else {
            // OGG: decode with stb_vorbis to interleaved 16-bit PCM
            int channels = 0;
            int sampleRate = 0;
            short* pcm = nullptr;
            const int samplesPerChannel = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &pcm);
            if (samplesPerChannel <= 0 || !pcm) {
                Logger::Err("[AudioSystem] Failed to load OGG '" + path + "'");
                continue;
            }
            data.spec.format = SDL_AUDIO_S16LE;
            data.spec.channels = channels;
            data.spec.freq = sampleRate;
            const size_t totalSamples = static_cast<size_t>(samplesPerChannel) * static_cast<size_t>(channels);
            const size_t byteLen = totalSamples * sizeof(short);
            data.buffer.assign(reinterpret_cast<Uint8*>(pcm), reinterpret_cast<Uint8*>(pcm) + byteLen);
            free(pcm);
        }

        g_sounds[id] = std::move(data);
        loaded++;
    }

    Logger::Log("[AudioSystem] Loaded " + std::to_string(loaded) +
                " sound(s) from '" + directory + "'");
}

void PlaySound(const SoundId& id, float volume) {
    if (!g_device) return;

    auto it = g_sounds.find(id);
    if (it == g_sounds.end()) {
        Logger::Err("[AudioSystem] PlaySound: unknown id '" + id + "'");
        return;
    }

    const SoundData& sound = it->second;

    // Use same spec for src and dst — SDL adjusts to device format on bind.
    SDL_AudioStream* stream = SDL_CreateAudioStream(&sound.spec, &sound.spec);
    if (!stream) {
        Logger::Err("[AudioSystem] SDL_CreateAudioStream failed: " + std::string(SDL_GetError()));
        return;
    }

    SDL_SetAudioStreamGain(stream, g_sfxVolume * volume);
    SDL_PutAudioStreamData(stream, sound.buffer.data(), static_cast<int>(sound.buffer.size()));
    // Flush tells SDL the data is complete so it outputs the final conversion chunk.
    SDL_FlushAudioStream(stream);

    if (!SDL_BindAudioStream(g_device, stream)) {
        Logger::Err("[AudioSystem] SDL_BindAudioStream failed: " + std::string(SDL_GetError()));
        SDL_DestroyAudioStream(stream);
        return;
    }

    g_sfxStreams.push_back(stream);
}

void PlayMusic(const SoundId& id) {
    if (!g_device) return;

    auto it = g_sounds.find(id);
    if (it == g_sounds.end()) {
        Logger::Err("[AudioSystem] PlayMusic: unknown id '" + id + "'");
        return;
    }

    // Stop any existing music
    if (g_musicStream) {
        SDL_DestroyAudioStream(g_musicStream);
        g_musicStream = nullptr;
    }

    const SoundData& music = it->second;

    g_musicStream = SDL_CreateAudioStream(&music.spec, &music.spec);
    if (!g_musicStream) {
        Logger::Err("[AudioSystem] SDL_CreateAudioStream (music) failed: " + std::string(SDL_GetError()));
        return;
    }

    SDL_SetAudioStreamGain(g_musicStream, g_musicVolume);

    // Pre-fill two loops so the first refill in Update() has headroom
    SDL_PutAudioStreamData(g_musicStream, music.buffer.data(), static_cast<int>(music.buffer.size()));
    SDL_PutAudioStreamData(g_musicStream, music.buffer.data(), static_cast<int>(music.buffer.size()));

    if (!SDL_BindAudioStream(g_device, g_musicStream)) {
        Logger::Err("[AudioSystem] SDL_BindAudioStream (music) failed: " + std::string(SDL_GetError()));
        SDL_DestroyAudioStream(g_musicStream);
        g_musicStream = nullptr;
        return;
    }

    g_currentMusic  = id;
    g_musicPlaying  = true;
    g_musicPaused   = false;
}

void StopMusic() {
    if (!g_musicStream) return;

    SDL_DestroyAudioStream(g_musicStream); // auto-unbinds
    g_musicStream  = nullptr;
    g_currentMusic.clear();
    g_musicPlaying = false;
    g_musicPaused  = false;
}

void PauseMusic() {
    if (!g_musicStream || g_musicPaused) return;
    // Unbind from device — data stays in the stream buffer.
    SDL_UnbindAudioStream(g_musicStream);
    g_musicPaused = true;
}

void ResumeMusic() {
    if (!g_musicStream || !g_musicPaused) return;
    // Rebind to device — continues from where it left off.
    SDL_BindAudioStream(g_device, g_musicStream);
    g_musicPaused = false;
}

bool IsMusicPlaying() {
    return g_musicPlaying && !g_musicPaused;
}

void SetMasterVolume(float volume) {
    g_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    // While muted the device gain stays at 0; apply the new value once unmuted.
    if (!g_muted && g_device) SDL_SetAudioDeviceGain(g_device, g_masterVolume);
}

void SetMuted(bool muted) {
    if (muted == g_muted) return;
    g_muted = muted;
    if (g_device) SDL_SetAudioDeviceGain(g_device, g_muted ? 0.f : g_masterVolume);
}

bool IsMuted() { return g_muted; }

void SetSfxVolume(float volume) {
    g_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    for (auto* s : g_sfxStreams) {
        SDL_SetAudioStreamGain(s, g_sfxVolume);
    }
}

void SetMusicVolume(float volume) {
    g_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    if (g_musicStream) SDL_SetAudioStreamGain(g_musicStream, g_musicVolume);
}

float GetMasterVolume() { return g_masterVolume; }
float GetSfxVolume()    { return g_sfxVolume; }
float GetMusicVolume()  { return g_musicVolume; }

} // namespace AudioSystem
