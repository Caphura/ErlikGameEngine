// src/engine/Animator.h
#pragma once
#include <algorithm>

namespace Erlik {
    class Animator {
    public:
        // Yeni API: belirli aralýðý oynat
        void setRange(int start, int count, float fps, bool loop = true) {
            m_start = std::max(0, start);
            m_count = std::max(1, count);
            m_fps = fps > 0 ? fps : 1.f;
            m_loop = loop;
            m_time = 0.f; m_index = 0;
        }

        // Eski API ile uyumluluk (Application.cpp init'te kullanýlýyor)
        void set(int count, float fps, bool loop = true) {
            setRange(0, count, fps, loop);
        }

        void update(double dt) {
            m_time += (float)dt; const float frameDur = 1.0f / m_fps;
            while (m_time >= frameDur) {
                m_time -= frameDur; m_index++;
                if (m_index >= m_count) { if (m_loop) m_index = 0; else { m_index = m_count - 1; break; } }
            }
        }

        int   index() const { return m_start + m_index; }
        void  setFPS(float f) { if (f > 0) m_fps = f; }
        float fps() const { return m_fps; }

    private:
        int   m_start = 0, m_count = 1, m_index = 0;
        float m_fps = 8.f;
        bool  m_loop = true;
        float m_time = 0.f;
    };
} // namespace Erlik
