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

        // �alma (klasik)
        int  playSfx(const std::string& name, int loops = 0, int channel = -1, int volume = -1);
        bool playMusic(const std::string& name, int loops = -1, float volume = 1.0f);
        bool playMusicFade(const std::string& name, int loops, int fadeMs, float volume = 1.0f);
        void stopMusic(int fadeMs = 0);

        // Ses d�zeyi
        void setSfxVolume(int v_0to128);
        void setMusicVolume(float v_0to1);
        bool isReady();

        // --- Crossfade API ---
        // E�er halihaz�rda m�zik �al�yorsa, �nce onu fade-out eder, bitti�inde 'name'i fade-in ile ba�lat�r.
        // E�er m�zik �alm�yorsa direkt fade-in ile ba�lat�r.
            bool crossfadeTo(const std::string & name,
                int loops = -1,
                int fadeOutMs = 250,
                int fadeInMs = 400,
                float volume = 1.0f);
        // Her frame �a��r: fade-out bitti mi diye bakar ve gerekiyorsa pending m�zi�i ba�lat�r.
            void tick();
            // Debug / durum
            std::string currentMusicName();  // o an �alan (veya bo�)
    }
}
