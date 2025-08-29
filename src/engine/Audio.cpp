#include "Audio.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <unordered_map>
#include <algorithm> // clamp
#include <iostream>

namespace {
    bool g_ready = false;
    std::unordered_map<std::string, Mix_Chunk*> g_sfx;
    std::unordered_map<std::string, Mix_Music*> g_music;
    int g_sfxVolume = MIX_MAX_VOLUME; // 0..128
    static std::string g_curMusic;
    static std::string g_nextMusic;
    static int   g_nextLoops = -1;
    static int   g_nextFadeIn = 0;
    static float g_nextVolume = 1.0f;

    enum class MusicState { Idle, FadingOut };
    static MusicState g_musicState = MusicState::Idle;

}

namespace Erlik {
    namespace Audio {

        bool init() {
            if (g_ready) return true;

            int want = MIX_INIT_OGG | MIX_INIT_MP3;
            int got = Mix_Init(want);
            if ((got & want) != want) {
                SDL_Log("[audio] Mix_Init warn: %s", Mix_GetError()); // mp3/ogg yoksa sadece uyar
            }
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
                SDL_Log("[audio] Mix_OpenAudio failed: %s", Mix_GetError());
                return false;
            }
            Mix_AllocateChannels(32);
            g_ready = true;
            return true;
        }

        void shutdown() {
            for (auto& kv : g_sfx) { Mix_FreeChunk(kv.second); }
            for (auto& kv : g_music) { Mix_FreeMusic(kv.second); }
            g_sfx.clear(); g_music.clear();

            if (g_ready) {
                Mix_CloseAudio();
                Mix_Quit();
                g_ready = false;
            }
        }

        bool loadSfx(const std::string& name, const std::string& path) {
            if (!g_ready) return false;
            Mix_Chunk* c = Mix_LoadWAV(path.c_str());
            if (!c) { SDL_Log("[audio] SFX load failed '%s': %s", path.c_str(), Mix_GetError()); return false; }
            g_sfx[name] = c;
            return true;
        }

        bool loadMusic(const std::string& name, const std::string& path) {
            if (!g_ready) return false;
            Mix_Music* m = Mix_LoadMUS(path.c_str());
            if (!m) { SDL_Log("[audio] Music load failed '%s': %s", path.c_str(), Mix_GetError()); return false; }
            g_music[name] = m;
            return true;
        }

        bool hasMusic(const std::string& name) {
            return g_music.find(name) != g_music.end();
            
        }

        int playSfx(const std::string& name, int loops, int channel, int volume) {
            auto it = g_sfx.find(name);
            if (it == g_sfx.end()) { SDL_Log("[audio] SFX not found '%s'", name.c_str()); return -1; }
            Mix_Chunk* c = it->second;
            Mix_VolumeChunk(c, volume >= 0 ? volume : g_sfxVolume);
            return Mix_PlayChannel(channel, c, loops);
        }

        bool playMusic(const std::string& name, int loops, float volume) {
            auto it = g_music.find(name);
            if (it == g_music.end()) { SDL_Log("[audio] Music not found '%s'", name.c_str()); return false; }
            Mix_VolumeMusic((int)(std::clamp(volume, 0.0f, 1.0f) * MIX_MAX_VOLUME));

            if (Mix_PlayMusic(it->second, loops) != 0) {
                SDL_Log("[audio] PlayMusic failed: %s", Mix_GetError());
                return false;
            }
            g_curMusic = name;
            return true;
        }

        bool playMusicFade(const std::string& name, int loops, int fadeMs, float volume) {
            auto it = g_music.find(name);
            if (it == g_music.end()) { SDL_Log("[audio] Music not found '%s'", name.c_str()); return false; }
            Mix_VolumeMusic((int)(std::clamp(volume, 0.0f, 1.0f) * MIX_MAX_VOLUME));
            if (Mix_FadeInMusic(it->second, loops, std::max(0, fadeMs)) != 0) {
                SDL_Log("[audio] FadeInMusic failed: %s", Mix_GetError());
                return false;
            }
            g_curMusic = name;
            return true;
        }

        void stopMusic(int fadeMs) {
            if (fadeMs > 0) Mix_FadeOutMusic(fadeMs);
            else            Mix_HaltMusic();
            g_musicState = MusicState::Idle;
            g_nextMusic.clear();
            g_curMusic.clear();

        }

        void setSfxVolume(int v) { g_sfxVolume = std::clamp(v, 0, MIX_MAX_VOLUME); }
        void setMusicVolume(float v) { Mix_VolumeMusic((int)(std::clamp(v, 0.0f, 1.0f) * MIX_MAX_VOLUME)); }
        bool isReady() { return g_ready; }

    }

    bool Erlik::Audio::crossfadeTo(const std::string& name, int loops, int fadeOutMs, int fadeInMs, float volume) {
        if (!g_ready) return false;
        if (name.empty()) return false;
        if (g_curMusic == name && Mix_PlayingMusic() != 0) {
            // Aynı parça zaten çalıyor → sadece Volume’u güncelle
            setMusicVolume(volume);
            return true;
        }

        // Pending hedefi yaz
        g_nextMusic = name;
        g_nextLoops = loops;
        g_nextFadeIn = std::max(0, fadeInMs);
        g_nextVolume = std::clamp(volume, 0.0f, 1.0f);

        if (Mix_PlayingMusic() != 0) {
            // Şu an müzik var → fade-out başlat
            Mix_FadeOutMusic(std::max(0, fadeOutMs));
            g_musicState = MusicState::FadingOut;
            return true;
        }

        // Müzik yok → hemen fade-in ile başlat
        auto it = g_music.find(name);
        if (it == g_music.end()) return false;

        setMusicVolume(g_nextVolume);
        if (g_nextFadeIn > 0) Mix_FadeInMusic(it->second, loops, g_nextFadeIn);
        else                   Mix_PlayMusic(it->second, loops);
        g_curMusic.clear();         // Mix_PlayMusic hemen başlasa da, çalıyor garantisi bir tick sonra
        g_curMusic = name;
        g_musicState = MusicState::Idle;
        g_nextMusic.clear();
        return true;
    }

    void Erlik::Audio::tick() {
        if (g_musicState == MusicState::FadingOut) {
            // Fade-out bitmiş mi?
            if (Mix_PlayingMusic() == 0) {
                // Yeni parçayı başlat
                if (!g_nextMusic.empty()) {
                    auto it = g_music.find(g_nextMusic);
                    if (it != g_music.end()) {
                        setMusicVolume(g_nextVolume);
                        if (g_nextFadeIn > 0) Mix_FadeInMusic(it->second, g_nextLoops, g_nextFadeIn);
                        else                   Mix_PlayMusic(it->second, g_nextLoops);
                        g_curMusic = g_nextMusic;
                    }
                }
                g_nextMusic.clear();
                g_musicState = MusicState::Idle;
            }
        }
    }

    std::string Erlik::Audio::currentMusicName() { return g_curMusic; }

} // namespace Erlik::Audio
