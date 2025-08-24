#pragma once
#include <string>
#include "Animator.h"

namespace Erlik {

    enum class AnimState { Idle, Run, Jump, Fall, Land };

    struct AnimParams {
        bool  onGround = false;
        float vx = 0.f;       // yatay hýz (+sað)
        float vy = 0.f;       // dikey hýz (+aþaðý)
        bool  jumpTrigger = false;  // bu framede zýplama tetiklendi mi?
    };

    class AnimatorController {
    public:
        void bind(Animator* a) { m_anim = a; }
        void setClipNames(const std::string& idle,
            const std::string& run,
            const std::string& jump,
            const std::string& fall,
            const std::string& land)
        {
            m_idle = idle; m_run = run; m_jump = jump; m_fall = fall; m_land = land;
        }

        void update(double dt, const AnimParams& p);

        AnimState   state() const { return m_state; }
        const char* stateName() const;

    private:
        Animator* m_anim = nullptr;
        AnimState  m_state = AnimState::Idle;
        AnimState  m_prev = AnimState::Idle;
        double     m_timeInState = 0.0;

        std::string m_idle = "idle", m_run = "run", m_jump = "jump", m_fall = "fall", m_land = "land";

        void change(AnimState s, bool forceRestart = false);
    };

} // namespace Erlik
