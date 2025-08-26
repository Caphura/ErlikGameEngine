#include "Input.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace Erlik {

    // --- Klavye durum dizileri (0/1)
    static std::array<uint8_t, SDL_NUM_SCANCODES> s_keyDown{};
    static std::array<uint8_t, SDL_NUM_SCANCODES> s_keyPressed{};
    static std::array<uint8_t, SDL_NUM_SCANCODES> s_keyReleased{};

    std::vector<Input::Pad> Input::s_pads;


    void Input::initGamepads() {
        // GameController alt sistemi açýk deðilse aç
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
        // Buton etiketlerini (A/B/X/Y) platform etiketine göre kullan
        SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "1");
        // (Ýsteðe baðlý) mapping dosyan varsa yükle: assets/gamecontrollerdb.txt
        SDL_GameControllerAddMappingsFromFile("assets/gamecontrollerdb.txt");

        // Baþlangýçta takýlý olanlarý aç
        int nJoy = SDL_NumJoysticks();
        for (int i = 0; i < nJoy; ++i) {
            if (!SDL_IsGameController(i)) continue;
            SDL_GameController* c = SDL_GameControllerOpen(i);
            if (!c) continue;
            SDL_Joystick* j = SDL_GameControllerGetJoystick(c);
            Pad p; p.ctl = c; p.jid = SDL_JoystickInstanceID(j);
            s_pads.push_back(p);
        }
    }

    void Input::shutdownGamepads() {
        for (auto& p : s_pads) {
            if (p.ctl) SDL_GameControllerClose(p.ctl);
        }
        s_pads.clear();
    }

    Input::Pad* Input::primary() {
        return s_pads.empty() ? nullptr : &s_pads[0];
    }

    float Input::normAxis(Sint16 v, int deadzone) {
        int a = std::abs((int)v);
        if (a < deadzone) return 0.0f;
        float f = (a - deadzone) / (32767.0f - deadzone);
        // yumuþatma (kavis)
        f = std::clamp(f, 0.0f, 1.0f);
        f = f * f; // kavis
        return (v < 0) ? -f : f;
    }

    // Event akýþýnda çaðýr
    void Input::handleControllerHotplug(const SDL_Event& e) {
        if (e.type == SDL_CONTROLLERDEVICEADDED) {
            int which = e.cdevice.which;
            if (!SDL_IsGameController(which)) return;
            SDL_GameController* c = SDL_GameControllerOpen(which);
            if (!c) return;
            SDL_Joystick* j = SDL_GameControllerGetJoystick(c);
            Pad p; p.ctl = c; p.jid = SDL_JoystickInstanceID(j);
            s_pads.push_back(p);
        }
        else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
            SDL_JoystickID jid = e.cdevice.which;
            for (auto it = s_pads.begin(); it != s_pads.end(); ++it) {
                if (it->jid == jid) {
                    if (it->ctl) SDL_GameControllerClose(it->ctl);
                    s_pads.erase(it);
                    break;
                }
            }
        }
    }

    void Input::beginFrame() {
        // KLAVYE anlýk bayraklarý her framede temizle
        s_keyPressed.fill(0);
        s_keyReleased.fill(0);

        // GAMEPAD anlýk bayraklarý
        for (auto& p : s_pads) {
            p.pressed.fill(0);
            p.released.fill(0);
        }
    }


    void Input::handleEvent(const SDL_Event& e) {
        // ---- KLAVYE ----
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            const SDL_Scancode sc = e.key.keysym.scancode;
            if (sc >= 0 && sc < SDL_NUM_SCANCODES) {
                if (e.type == SDL_KEYDOWN) {
                    if (!s_keyDown[sc] && e.key.repeat == 0) s_keyPressed[sc] = 1;
                    s_keyDown[sc] = 1;
                }
                else { // KEYUP
                    if (s_keyDown[sc]) s_keyReleased[sc] = 1;
                    s_keyDown[sc] = 0;
                }
            }
        }

        // ---- GAMEPAD BUTON ----
        if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP) {
            SDL_JoystickID jid = e.cbutton.which;
            for (auto& p : s_pads) {
                if (p.jid != jid) continue;
                auto b = (SDL_GameControllerButton)e.cbutton.button;
                uint8_t isDown = (e.type == SDL_CONTROLLERBUTTONDOWN) ? 1u : 0u;
                if (p.down[b] != isDown) {
                    if (isDown) p.pressed[b] = 1;
                    else        p.released[b] = 1;
                }
                p.down[b] = isDown;
                break;
            }
        }
        else if (e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED) {
            handleControllerHotplug(e);
        }
    } // <<< ÖNEMLÝ: handleEvent burada KAPANIYOR


    // ==== KLAVYE SORGULARI (handleEvent DIÞINDA) ====
    bool Input::keyDown(SDL_Scancode sc) {
        return (sc >= 0 && sc < SDL_NUM_SCANCODES) ? (s_keyDown[sc] != 0) : false;
    }
    bool Input::keyPressed(SDL_Scancode sc) {
        return (sc >= 0 && sc < SDL_NUM_SCANCODES) ? (s_keyPressed[sc] != 0) : false;
    }
    bool Input::keyReleased(SDL_Scancode sc) {
        return (sc >= 0 && sc < SDL_NUM_SCANCODES) ? (s_keyReleased[sc] != 0) : false;
    }


    // ==== GAMEPAD SORGULARI ====
    bool Input::padButtonDown(SDL_GameControllerButton b) {
        Pad* p = primary(); if (!p) return false; return p->down[b] != 0;
    }
    bool Input::padButtonPressed(SDL_GameControllerButton b) {
        Pad* p = primary(); if (!p) return false; return p->pressed[b] != 0;
    }
    float Input::padAxisLX() {
        Pad* p = primary(); if (!p) return 0.f;
        return normAxis(SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_LEFTX));
    }
    float Input::padAxisLY() {
        Pad* p = primary(); if (!p) return 0.f;
        return normAxis(SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_LEFTY));
    }
    float Input::padAxisRX() {
        Pad* p = primary(); if (!p) return 0.f;
        return normAxis(SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_RIGHTX));
    }
    float Input::padAxisRY() {
        Pad* p = primary(); if (!p) return 0.f;
        return normAxis(SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_RIGHTY));
    }
    float Input::padTrigL() {
        Pad* p = primary(); if (!p) return 0.f;
        Sint16 v = SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        return std::clamp(v / 32767.0f, 0.0f, 1.0f);
    }
    float Input::padTrigR() {
        Pad* p = primary(); if (!p) return 0.f;
        Sint16 v = SDL_GameControllerGetAxis(p->ctl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        return std::clamp(v / 32767.0f, 0.0f, 1.0f);
    }
    void Input::rumble(uint16_t low, uint16_t high, uint32_t ms) {
        Pad* p = primary(); if (!p) return;
        SDL_GameControllerRumble(p->ctl, low, high, ms);
    }


} // namespace Erlik
