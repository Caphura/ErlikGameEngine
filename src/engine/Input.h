#pragma once
#include <vector>
#include <SDL.h>

namespace Erlik {

class Input {
public:
    static void init();
    static void shutdown();
    static void beginFrame();
    static void endFrame();

    static bool keyDown(SDL_Scancode sc);
    static bool keyPressed(SDL_Scancode sc);
    static bool keyReleased(SDL_Scancode sc);

private:
    static inline const Uint8* s_currKeys = nullptr;
    static inline std::vector<Uint8> s_prevKeys;
    static inline int s_keyCount = 0;
};

} // namespace Erlik