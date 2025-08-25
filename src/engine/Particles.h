#pragma once
#include <SDL.h>
#include <algorithm>
#include <cstdlib>

namespace Erlik {

    struct Particle {
        float x = 0, y = 0, vx = 0, vy = 0, size = 6;
        float life = 0, maxLife = 0.5f;
        float gravityScale = 1.0f;   // YENÝ: her parçacýk için yerçekimi katsayýsý
        float drag = 2.5f;   // YENÝ: yatay fren katsayýsý
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
        // iniþte toz — dir: +1 saða, -1 sola; baseSpeed: baþlangýç yatay hýz
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
