#pragma once
#include <SDL.h>
#include <algorithm>
#include <cstdlib>

namespace Erlik {

    struct Particle {
        float x = 0, y = 0, vx = 0, vy = 0, size = 6;
        float life = 0, maxLife = 0.5f;
        float gravityScale = 1.0f;   // YEN�: her par�ac�k i�in yer�ekimi katsay�s�
        float drag = 2.5f;   // YEN�: yatay fren katsay�s�
        Uint8  baseA = 255;
        bool   alive = false;
    };

    class Renderer2D; // ileri bildirim

    class ParticleSystem {
    public:
        void init(int cap = 256) { m_cap = std::min(cap, MAX_CAP); clear(); }
        void clear() { for (int i = 0; i < MAX_CAP; ++i) m_pool[i].alive = false; m_next = 0; }
        void update(float dt);
        void draw(Renderer2D& r2d) const;
        void emitFootDust(float x, float y, int count, float dir);
        // ini�te toz � dir: +1 sa�a, -1 sola; baseSpeed: ba�lang�� yatay h�z
        void emitDust(float x, float y, int count, float dir, float baseSpeed);

    private:
        static constexpr int MAX_CAP = 256;
        Particle m_pool[MAX_CAP];
        int m_cap = MAX_CAP;
        int m_next = 0;

        static inline float frand(float a, float b) {
            return a + (b - a) * (float(std::rand()) / float(RAND_MAX));
        }
    };

} // namespace Erlik
