#include "Input.h"
#include <cstring>

namespace Erlik {

    void Input::init() {
        s_currKeys = SDL_GetKeyboardState(&s_keyCount);
        s_prevKeys.assign(s_keyCount, 0);
        s_prevMouse = 0;
        s_currMouse = SDL_GetMouseState(&s_mx, &s_my);
    }

    void Input::shutdown() {}

    void Input::beginFrame() {
        if (!s_currKeys) {
            s_currKeys = SDL_GetKeyboardState(&s_keyCount);
            s_prevKeys.assign(s_keyCount, 0);
        }
        std::memcpy(s_prevKeys.data(), s_currKeys, static_cast<size_t>(s_keyCount));
        s_prevMouse = s_currMouse;
    }

    void Input::endFrame() {
        int n = 0;
        s_currKeys = SDL_GetKeyboardState(&n);
        if (n != s_keyCount) {
            s_keyCount = n;
            s_prevKeys.resize(s_keyCount, 0);
        }
        s_currMouse = SDL_GetMouseState(&s_mx, &s_my);
    }

    bool Input::keyDown(SDL_Scancode sc) {
        return s_currKeys && s_currKeys[sc] != 0;
    }
    bool Input::keyPressed(SDL_Scancode sc) {
        return s_currKeys && (s_currKeys[sc] != 0) && (s_prevKeys[sc] == 0);
    }
    bool Input::keyReleased(SDL_Scancode sc) {
        return s_currKeys && (s_currKeys[sc] == 0) && (s_prevKeys[sc] != 0);
    }

    int  Input::mouseX() { return s_mx; }
    int  Input::mouseY() { return s_my; }

    bool Input::mouseDown(Uint32 button) {
        return (s_currMouse & SDL_BUTTON(button)) != 0;
    }
    bool Input::mousePressed(Uint32 button) {
        return ((s_currMouse & SDL_BUTTON(button)) != 0) &&
            ((s_prevMouse & SDL_BUTTON(button)) == 0);
    }
    bool Input::mouseReleased(Uint32 button) {
        return ((s_currMouse & SDL_BUTTON(button)) == 0) &&
            ((s_prevMouse & SDL_BUTTON(button)) != 0);
    }

} // namespace Erlik
