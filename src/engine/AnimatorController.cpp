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

    float AnimatorController::runSpeedFactor(float ax)
    {
        // Map velocity to fps for run clip (tune freely)
        // 0 -> 6 fps, 100 -> 8 fps, 200 -> 10 fps, clamp 14 fps
        float fps = 6.f + (ax / 100.f) * 2.f;
        if (ax >= 200.f) fps = 10.f + (ax - 200.f) * 0.02f;
        return std::clamp(fps, 6.f, 14.f);
    }

    void AnimatorController::change(AnimState s, bool forceRestart)
    {
        if (m_state == s && !forceRestart) return;
        m_prev = m_state;
        m_state = s;
        m_timeInState = 0.0;

        if (!m_anim) return;
        switch (m_state) {
        case AnimState::Idle: m_anim->play(m_idle, forceRestart);  m_anim->setFPS(6.f);  break;
        case AnimState::Run:  m_anim->play(m_run, forceRestart);   /* fps set per update */ break;
        case AnimState::Jump: m_anim->play(m_jump, forceRestart);  m_anim->setFPS(12.f); break;
        case AnimState::Fall: m_anim->play(m_fall, forceRestart);  m_anim->setFPS(10.f); break;
        case AnimState::Land: m_anim->play(m_land, forceRestart);  m_anim->setFPS(12.f); break;
        }
    }

    void AnimatorController::update(const AnimParams& p, float dt)
    {
        if (!m_anim) return;
        m_timeInState += dt;

        // Jump trigger takes priority
        if (p.jumpTrigger) {
            change(AnimState::Jump, true);
            return;
        }

        switch (m_state) {
        case AnimState::Idle:
        case AnimState::Run: {
            if (!p.onGround) {
                change((p.vy < 0.f) ? AnimState::Jump : AnimState::Fall, true);
            }
            else {
                if (absf(p.vx) > 30.f) {
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
            if (p.vy >= 0.f) change(AnimState::Fall, true);
        } break;

        case AnimState::Fall: {
            if (p.onGround) {
                change(AnimState::Land, true);
            }
        } break;

        case AnimState::Land: {
            constexpr float LAND_MIN_SEC = 0.06f;
            if (m_timeInState >= LAND_MIN_SEC) {
                if (absf(p.vx) > 30.f) {
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
