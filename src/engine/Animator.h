#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Erlik {

    class Animator {
    public:
        struct Clip {
            int   start = 0;
            int   count = 1;
            float fps = 8.f;
            bool  loop = true;
        };

        // --- Clips ---
        void addClip(const std::string& name, int start, int count, float fps = 8.f, bool loop = true);
        bool play(const std::string& name, bool restart = false);

        // --- Playback ---
        void setFPS(float fps) { m_fps = fps; }
        float fps() const { return m_fps; }

        void setTotalFrames(int total) { m_totalFrames = total; }
        void setIndex(int idx);
        int  index() const { return m_index; }

        void update(float dt);
        const std::string& currentClip() const { return m_clipName; }

    private:
        // Timeline
        double m_time = 0.0;
        float  m_fps = 8.f;
        int    m_index = 0;

        // Active clip
        std::unordered_map<std::string, Clip> m_clips;
        std::string m_clipName;
        int   m_clipStart = 0;
        int   m_clipCount = 1;
        bool  m_clipLoop = true;

        // Optional total frame count for clamping (e.g., SpriteAtlas::frameCount())
        int   m_totalFrames = 0;
    };

} // namespace Erlik
