#pragma once
#include <string>
#include "Animator.h"

namespace Erlik {

    enum class AnimState { Idle, Run, Jump, Fall, Land };

    struct AnimParams {
        bool  onGround = false;
        float vx = 0.f;        // +right
        float vy = 0.f;        // +down
        bool  jumpTrigger = false;
    };

    class AnimatorController {
    public:
        void attach(Animator& anim) { m_anim = &anim; }
        void bind(Animator* anim) { m_anim = anim; }
        void setClipNames(const std::string& idle, const std::string& run,
            const std::string& jump, const std::string& fall,
            const std::string& land)
        {
            m_idle = idle; m_run = run; m_jump = jump; m_fall = fall; m_land = land;
        }

        void update(const AnimParams& p, float dt);
        const char* stateName() const;

    private:
        Animator* m_anim = nullptr;
        AnimState  m_state = AnimState::Idle;
        AnimState  m_prev = AnimState::Idle;
        double     m_timeInState = 0.0;

        std::string m_idle = "idle", m_run = "run", m_jump = "jump", m_fall = "fall", m_land = "land";

        void change(AnimState s, bool forceRestart = false);
        static float runSpeedFactor(float absVx);
    };

} // namespace Erlik
