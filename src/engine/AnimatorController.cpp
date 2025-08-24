#include "AnimatorController.h"
#include <algorithm>
#include <cmath>

namespace Erlik {

    static inline float absf(float x) { return x < 0 ? -x : x; }

    const char* AnimatorController::stateName() const {
        switch (m_state) {
        case AnimState::Idle: return "Idle";
        case AnimState::Run:  return "Run";
        case AnimState::Jump: return "Jump";
        case AnimState::Fall: return "Fall";
        case AnimState::Land: return "Land";
        default: return "?";
        }
    }

    void AnimatorController::change(AnimState s, bool forceRestart) {
        if (!m_anim) return;
        if (s == m_state && !forceRestart) return;
        m_prev = m_state;
        m_state = s;
        m_timeInState = 0.0;

        switch (s) {
        case AnimState::Idle: m_anim->play(m_idle, forceRestart); break;
        case AnimState::Run:  m_anim->play(m_run, forceRestart); break;
        case AnimState::Jump: m_anim->play(m_jump,  /*force*/true); break; // one-shot
        case AnimState::Fall: m_anim->play(m_fall, forceRestart); break;
        case AnimState::Land: m_anim->play(m_land,  /*force*/true); break;
        }
    }

    void AnimatorController::update(double dt, const AnimParams& p) {
        if (!m_anim) return;
        m_timeInState += dt;

        // Eþikler
        const float RUN_THRESH = 20.f;   // koþu eþiði (|vx|)
        const float RUN_FPS_MIN = 4.f;    // koþu fps aralýðý
        const float RUN_FPS_MAX = 16.f;
        const float LAND_MIN_SEC = 0.12f;  // Land süresi

        // Hýz tabanlý fps: 0..1 arasý
        auto runSpeedFactor = [&](float vxAbs)->float {
            // “koþu eþiði”ne ulaþýnca 1.0 sayalým (istersen m_pp.moveSpeed ile ölçekleyebilirsin)
            float k = std::clamp(vxAbs / (RUN_THRESH * 2.f), 0.f, 1.f);
            return RUN_FPS_MIN + (RUN_FPS_MAX - RUN_FPS_MIN) * k;
        };

        switch (m_state) {
        case AnimState::Idle:
        case AnimState::Run: {
            if (!p.onGround) {
                change(AnimState::Fall);
            }
            else if (p.jumpTrigger) {
                change(AnimState::Jump);
            }
            else {
                if (absf(p.vx) > RUN_THRESH) {
                    change(AnimState::Run);
                    m_anim->setFPS(runSpeedFactor(absf(p.vx)));
                }
                else {
                    change(AnimState::Idle);
                    m_anim->setFPS(6.f);
                }
            }
        } break;

        case AnimState::Jump: {
            // havaya çýktýysan ve artýk aþaðý dönüyorsan Fall
            if (!p.onGround && p.vy > 0.f) change(AnimState::Fall);
            else if (p.onGround)          change(AnimState::Land);
        } break;

        case AnimState::Fall: {
            if (p.onGround) change(AnimState::Land);
        } break;

        case AnimState::Land: {
            if (m_timeInState >= LAND_MIN_SEC) {
                if (absf(p.vx) > RUN_THRESH) {
                    change(AnimState::Run);
                    m_anim->setFPS(runSpeedFactor(absf(p.vx)));
                }
                else {
                    change(AnimState::Idle);
                    m_anim->setFPS(6.f);
                }
            }
        } break;
        }
    }

} // namespace Erlik
