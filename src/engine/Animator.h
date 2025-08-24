#pragma once
#include <string>
#include <unordered_map>

namespace Erlik {

    class Animator {
    public:
        struct Clip {
            int   start = 0;
            int   count = 1;
            float fps = 8.f;
            bool  loop = true;
        };

        // Klip yönetimi
        void addClip(const std::string& name, int start, int count, float fps, bool loop);
        bool play(const std::string& name, bool forceRestart = false);
        const std::string& currentClip() const { return m_clipName; }

        // Zaman/çerçeve
        void  update(double dt);
        int   index() const;
        float fps() const;
        void  setFPS(float f);
        void  setTotalFrames(int n) { m_totalFrames = n; }

    private:
        // Çalýþma durumu
        int    m_index = 0;     // mutlak frame index (atlas karesine direkt gider)
        double m_time = 0.0;
        float  m_fps = 8.f;

        // Aktif klip bilgisi
        std::unordered_map<std::string, Clip> m_clips;
        std::string m_clipName;
        int   m_clipStart = 0;
        int   m_clipCount = 1;
        bool  m_clipLoop = true;

        // Opsiyonel toplam frame sayýsý (clamp için)
        int   m_totalFrames = 0;
    };

} // namespace Erlik
