#pragma once
#include <vector>
#include <SDL.h>

namespace Erlik {

    class Input {
    public:
        static void init();
        static void shutdown();

        // Sýra: beginFrame() -> (SDL_PollEvent/SDL_PumpEvents) -> endFrame()
        static void beginFrame();
        static void endFrame();

        static bool keyDown(SDL_Scancode sc);
        static bool keyPressed(SDL_Scancode sc);
        static bool keyReleased(SDL_Scancode sc);

        static int  mouseX();
        static int  mouseY();
        static bool mouseDown(Uint32 button);
        static bool mousePressed(Uint32 button);
        static bool mouseReleased(Uint32 button);

    private:
        static inline const Uint8* s_currKeys = nullptr;
        static inline std::vector<Uint8> s_prevKeys;
        static inline int s_keyCount = 0;

        static inline int s_mx = 0, s_my = 0;
        static inline Uint32 s_currMouse = 0;
        static inline Uint32 s_prevMouse = 0;
    };

} // namespace Erlik
