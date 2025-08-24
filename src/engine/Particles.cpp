#include "Particles.h"
#include "Renderer2D.h"
#include <cmath>

namespace Erlik {

    void ParticleSystem::update(float dt)
    {
        const float gravity = 900.f;   // toz yere doðru çöker
        const float drag = 2.5f;    // yatay fren

        for (int i = 0; i < m_cap; ++i) {
            auto& p = m_pool[i];
            if (!p.alive) continue;

            p.life += dt;
            if (p.life >= p.maxLife) { p.alive = false; continue; }

            // hareket
            p.vy += gravity * dt;
            p.vx -= p.vx * drag * dt;        // basit sürtünme
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            // boyutu hafif küçült
            p.size = std::max(0.f, p.size - 20.f * dt);
        }
    }

    void ParticleSystem::draw(Renderer2D& r2d) const
    {
        for (int i = 0; i < m_cap; ++i) {
            const auto& p = m_pool[i];
            if (!p.alive) continue;

            float t = p.life / p.maxLife;          // 0..1
            Uint8 a = Uint8(std::max(0.f, (1.f - t) * float(p.baseA)));

            SDL_Color c{ 220, 220, 200, a };
            float s = std::max(1.f, p.size);
            r2d.fillRect(p.x - s * 0.5f, p.y - s * 0.5f, s, s, c);
        }
    }

    void ParticleSystem::emitDust(float x, float y, int count, float dir, float baseSpeed)
    {
        count = std::max(1, std::min(count, 32));
        for (int n = 0; n < count; ++n) {
            auto& p = m_pool[m_next];
            m_next = (m_next + 1) % m_cap;

            p.alive = true;
            p.x = x + frand(-6.f, 6.f);
            p.y = y + frand(-3.f, 2.f);
            p.vx = (baseSpeed + frand(-60.f, 60.f)) * dir * frand(0.6f, 1.0f);
            p.vy = frand(-220.f, -120.f);          // önce hafif yukarý
            p.size = frand(5.f, 9.f);
            p.life = 0.f;
            p.maxLife = frand(0.35f, 0.6f);
            p.baseA = 230;
        }
    }

} // namespace Erlik
