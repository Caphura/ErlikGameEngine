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

            // hareket: her parçacýðýn kendi katsayýlarý
            p.vy += (baseGravity * p.gravityScale) * dt;
            p.vx -= p.vx * p.drag * dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;

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
            // daha bulutumsu: daha az hýz, daha fazla sürtünme, biraz daha iri
            p.vx = (baseSpeed * frand(0.5f, 0.9f) + frand(-40.f, 40.f)) * dir;
            p.vy = frand(-180.f, -90.f);
            p.size = frand(6.f, 10.f);
            p.life = 0.f;
            p.maxLife = frand(0.40f, 0.70f);
            p.baseA = 220;
            p.gravityScale = 0.8f;   // daha yavaþ çöksün
            p.drag = 5.5f;   // yatayda çabuk sönsün

        }
    }

    void ParticleSystem::emitFootDust(float x, float y, int count, float dir)
    {
        count = std::max(1, std::min(count, 12));
        for (int n = 0; n < count; ++n) {
            auto& p = m_pool[m_next];
            m_next = (m_next + 1) % m_cap;

            p.alive = true;
            p.x = x + frand(-4.f, 4.f);
            p.y = y + frand(-2.f, 2.f);

            // daha YAVAÞ ve GERÝYE doðru
            p.vx = frand(20.f, 60.f) * dir;   // landing’e göre çok daha düþük
            p.vy = frand(-60.f, -20.f);       // hafif yukarý, yavaþ

            p.size = frand(4.f, 7.f);
            p.life = 0.f;
            p.maxLife = frand(0.28f, 0.48f);

            // toz: daha çabuk yatýþsýn, az yerçekimi
            p.gravityScale = 0.35f;           // landing 1.0 iken koþu 0.35
            p.drag = 6.0f;            // yatayda çabuk dur

            p.baseA = 210;
        }
    }

} // namespace Erlik
