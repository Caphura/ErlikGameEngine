#pragma once
#include <string>

namespace Erlik {
    namespace Audio {
        // Yaşam döngüsü
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
        // Eğer halihazırda müzik çalıyorsa, önce onu fade-out eder, bittiğinde 'name'i fade-in ile başlatır.
        // Eğer müzik çalmıyorsa direkt fade-in ile başlatır.
            bool crossfadeTo(const std::string & name,
                int loops = -1,
                int fadeOutMs = 250,
                int fadeInMs = 400,
                float volume = 1.0f);
        // Her frame çağır: fade-out bitti mi diye bakar ve gerekiyorsa pending müziği başlatır.
            void tick();
            // Debug / durum
            std::string currentMusicName();  // o an çalan (veya boş)

            // --- Positional SFX (pan + mesafe zayıflaması) ---
            // Listener konumunu ayarla (genelde oyuncu). Her kare bir kez çağır.
            void setListener(float x, float y);
            // Kaynak konumundan çal: X/Y ve bir "duyma yarıçapı" (piksel) ver.
            // Mesafe [0..hearRadius] → ses, (1-d)^2 ile kısılır; pan X farkından türetilir.
            int  playSfxAt(const std::string & name,
            float srcX, float srcY,
            float hearRadiusPx = 600.f,
            int baseVolume = -1);

            // Doğrudan pan/dist ile çal (pan01: 0=sol, 0.5=orta, 1=sağ, dist01: 0=yakın, 1=uzak)
            int  playSfxPan(const std::string & name,
            float pan01, float dist01,
            int baseVolume = -1);
    }
}
