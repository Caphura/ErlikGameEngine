#pragma once
#include <string>

namespace Erlik {
    namespace Audio {
        // Ya�am d�ng�s�
        bool init();
        void shutdown();

        // Y�kleme
        bool loadSfx(const std::string& name, const std::string& path);
        bool loadMusic(const std::string& name, const std::string& path);
        bool hasMusic(const std::string& name);

        // �alma
        int  playSfx(const std::string& name, int loops = 0, int channel = -1, int volume = -1);
        bool playMusic(const std::string& name, int loops = -1, float volume = 1.0f);
        bool playMusicFade(const std::string& name, int loops, int fadeMs, float volume = 1.0f);
        void stopMusic(int fadeMs = 0);

        // Ses d�zeyi
        void setSfxVolume(int v_0to128);
        void setMusicVolume(float v_0to1);
        bool isReady();
    }
}
