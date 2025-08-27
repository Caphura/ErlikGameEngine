#include "Animator.h"
#include <algorithm>
#include <cmath>

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

    bool Animator::play(const std::string& name, bool restart)
    {
        auto it = m_clips.find(name);
        if (it == m_clips.end()) return false;

        const Clip& c = it->second;
        const bool changing = (m_clipName != name);
        m_clipName = name;
        m_clipStart = c.start;
        m_clipCount = c.count;
        m_clipLoop = c.loop;
        m_fps = c.fps;

        if (changing || restart) {
            m_time = 0.0;
            m_index = m_clipStart;
        }
        else {
            // Keep index but clamp to the new clip range
            m_index = std::clamp(m_index, m_clipStart, m_clipStart + m_clipCount - 1);
        }
        return true;
    }

    void Animator::setIndex(int idx)
    {
        if (m_totalFrames > 0) {
            idx = std::clamp(idx, 0, m_totalFrames - 1);
        }
        m_index = idx;
    }

    void Animator::update(float dt)
    {
        if (m_clipCount <= 0) return;
        m_time += dt;

        const int frameAdvance = static_cast<int>(std::floor(m_time * m_fps));
        if (frameAdvance <= 0) return;

        // Consume the whole step (keeps remainder)
        m_time -= static_cast<double>(frameAdvance) / static_cast<double>(m_fps);

        int localIndex;
        if (m_clipLoop) {
            const int ofs = frameAdvance % m_clipCount;
            const int pos = (m_index - m_clipStart + ofs) % m_clipCount;
            localIndex = m_clipStart + pos;
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
