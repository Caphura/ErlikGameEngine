#include "Particles.h"
#include "Renderer2D.h"
#include <cmath>

namespace Erlik {

    void ParticleSystem::update(float dt)
    {
        const float baseGravity = 900.f;

        for (int i = 0; i < m_cap; ++i) {
            auto& p = m_pool[i];
            if (!p.alive) continue;

            p.life += dt;
            if (p.life >= p.maxLife) { p.alive = false; continue; }

            // Integrate
            p.vx *= std::max(0.f, 1.f - p.drag * dt);
            p.vy += baseGravity * p.gravityScale * dt;

            p.x += p.vx * dt;
            p.y += p.vy * dt;
        }
    }

    void ParticleSystem::draw(Renderer2D& r2d) const
    {
        for (int i = 0; i < m_cap; ++i) {
            const auto& p = m_pool[i];
            if (!p.alive) continue;

            const float t = p.life / std::max(0.0001f, p.maxLife);
            const Uint8 a = static_cast<Uint8>(std::clamp(1.f - t, 0.f, 1.f) * p.baseA);

            SDL_Color col{ 180, 180, 180, a };
            r2d.fillRect(p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size, col);
        }
    }

    void ParticleSystem::emitFootDust(float x, float y, int count, float dir)
    {
        emitDust(x, y, count, dir, 80.f);
    }

    void ParticleSystem::emitDust(float x, float y, int count, float dir, float baseSpeed)
    {
        dir = (dir >= 0.f) ? 1.f : -1.f;
        for (int i = 0; i < count; ++i) {
            Particle& p = m_pool[m_next];
            m_next = (m_next + 1) % m_cap;

            p.alive = true;
            p.x = x + frand(-2.f, 2.f);
            p.y = y + frand(-2.f, 2.f);
            p.vx = dir * (baseSpeed + frand(-20.f, 20.f));
            p.vy = frand(-60.f, -20.f);

            p.size = frand(4.f, 7.f);
            p.life = 0.f;
            p.maxLife = frand(0.28f, 0.48f);

            // dust: light, short-lived
            p.gravityScale = 0.35f;
            p.drag = 6.0f;

            p.baseA = 210;
        }
    }

} // namespace Erlik
