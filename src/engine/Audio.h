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

        // Çalma (klasik)
        int  playSfx(const std::string& name, int loops = 0, int channel = -1, int volume = -1);
        bool playMusic(const std::string& name, int loops = -1, float volume = 1.0f);
        bool playMusicFade(const std::string& name, int loops, int fadeMs, float volume = 1.0f);
        void stopMusic(int fadeMs = 0);

        // Ses düzeyi
        void setSfxVolume(int v_0to128);
        void setMusicVolume(float v_0to1);
        bool isReady();

        // --- Crossfade API ---
        // Eðer halihazýrda müzik çalýyorsa, önce onu fade-out eder, bittiðinde 'name'i fade-in ile baþlatýr.
        // Eðer müzik çalmýyorsa direkt fade-in ile baþlatýr.
            bool crossfadeTo(const std::string & name,
                int loops = -1,
                int fadeOutMs = 250,
                int fadeInMs = 400,
                float volume = 1.0f);
        // Her frame çaðýr: fade-out bitti mi diye bakar ve gerekiyorsa pending müziði baþlatýr.
            void tick();
            // Debug / durum
            std::string currentMusicName();  // o an çalan (veya boþ)
    }
}
