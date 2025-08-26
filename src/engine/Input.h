#pragma once
#include <SDL.h>
#include <array>
#include <vector>
#include <string>
#include <cstdint>

namespace Erlik {

    class Input {
    public:
        // — klavye API’n mevcut, dokunmuyoruz —
        static void beginFrame();
        static void handleEvent(const SDL_Event& e);
        static bool keyDown(SDL_Scancode sc);
        static bool keyPressed(SDL_Scancode sc);
        static bool keyReleased(SDL_Scancode sc);

        // -------- Gamepad API --------
        static void initGamepads();
        static void shutdownGamepads();
        static void handleControllerHotplug(const SDL_Event& e); // ADD/REMOVED eventi
        static bool padButtonDown(SDL_GameControllerButton b);
        static bool padButtonPressed(SDL_GameControllerButton b);
        static float padAxisLX();  // [-1..1]
        static float padAxisLY();  // [-1..1] (+ aşağı)
        static float padAxisRX();
        static float padAxisRY();
        static float padTrigL();   // [0..1]
        static float padTrigR();   // [0..1]
        static void rumble(uint16_t low, uint16_t high, uint32_t ms); // opsiyonel

    private:
        struct Pad {
            SDL_GameController* ctl = nullptr;
            SDL_JoystickID jid = -1;
            std::array<uint8_t, SDL_CONTROLLER_BUTTON_MAX> down{};    // 0/1
            std::array<uint8_t, SDL_CONTROLLER_BUTTON_MAX> pressed{}; // bu frame 1
            std::array<uint8_t, SDL_CONTROLLER_BUTTON_MAX> released{};
        };
        static std::vector<Pad> s_pads; // birden fazla kol → ilkini “primary” sayarız

        static Pad* primary();
        static float normAxis(Sint16 v, int deadzone = 8000); // yardımcı
    };

} // namespace Erlik
