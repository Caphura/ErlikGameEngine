#include "Animator.h"
#include <algorithm>  // std::clamp, std::min
#include <cmath>      // std::floor

namespace Erlik {

    void Animator::addClip(const std::string& name, int start, int count, float fps, bool loop)
    {
        if (count < 1) count = 1;
        if (start < 0) start = 0;

        if (m_totalFrames > 0) {
            start = std::min(start, m_totalFrames - 1);
            count = std::min(count, std::max(1, m_totalFrames - start));
        }
        m_clips[name] = Clip{ start, count, fps, loop };
    }

    bool Animator::play(const std::string& name, bool forceRestart)
    {
        auto it = m_clips.find(name);
        if (it == m_clips.end()) return false;

        if (name != m_clipName || forceRestart) {
            m_clipName = name;
            m_clipStart = it->second.start;
            m_clipCount = it->second.count;
            m_clipLoop = it->second.loop;
            setFPS(it->second.fps);

            m_time = 0.0;
            m_index = m_clipStart;
        }
        return true;
    }

    int Animator::index() const { return m_index; }

    float Animator::fps() const { return m_fps; }

    void Animator::setFPS(float f)
    {
        // min ~0.016fps (1 frame / 60s) , max 120fps
        m_fps = std::clamp(f, 1.0f / 60.0f, 120.0f);
    }

    void Animator::update(double dt)
    {
        if (m_fps <= 0.0f) return;

        m_time += dt;
        const int frameAdvance = (int)std::floor(m_time * (double)m_fps);

        if (m_clipCount <= 0) m_clipCount = 1;

        int localIndex = 0;
        if (m_clipLoop) {
            const int ofs = (m_clipCount > 0) ? (frameAdvance % m_clipCount) : 0;
            localIndex = m_clipStart + ofs;
        }
        else {
            const int ofs = std::min(frameAdvance, m_clipCount - 1);
            localIndex = m_clipStart + ofs;
        }

        if (m_totalFrames > 0) {
            if (localIndex < 0) localIndex = 0;
            if (localIndex >= m_totalFrames) localIndex = m_totalFrames - 1;
        }

        m_index = localIndex;
    }

} // namespace Erlik
