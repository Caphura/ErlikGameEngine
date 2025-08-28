#pragma once
#include <string>

namespace Erlik {
    namespace Audio {
        // Yaþam döngüsü
        bool init();
        void shutdown();

        // Yükleme
        bool loadSfx(const std::string& name, const std::string& path);
        bool loadMusic(const std::string& name, const std::string& path);
        bool hasMusic(const std::string& name);

        // Çalma
        int  playSfx(const std::string& name, int loops = 0, int channel = -1, int volume = -1);
        bool playMusic(const std::string& name, int loops = -1, float volume = 1.0f);
        bool playMusicFade(const std::string& name, int loops, int fadeMs, float volume = 1.0f);
        void stopMusic(int fadeMs = 0);

        // Ses düzeyi
        void setSfxVolume(int v_0to128);
        void setMusicVolume(float v_0to1);
        bool isReady();
    }
}
