#include "Input.h"
namespace Erlik {
void Input::init(){ s_currKeys = SDL_GetKeyboardState(&s_keyCount); s_prevKeys.assign(s_keyCount,0); }
void Input::shutdown(){}
void Input::beginFrame(){ if(!s_currKeys){ s_currKeys=SDL_GetKeyboardState(&s_keyCount); s_prevKeys.assign(s_keyCount,0);} std::memcpy(s_prevKeys.data(), s_currKeys, (size_t)s_keyCount); }
void Input::endFrame(){ int n=0; s_currKeys=SDL_GetKeyboardState(&n); if(n!=s_keyCount){ s_keyCount=n; s_prevKeys.resize(s_keyCount,0);} }
bool Input::keyDown(SDL_Scancode sc){ return s_currKeys && s_currKeys[sc]!=0; }
bool Input::keyPressed(SDL_Scancode sc){ return s_currKeys && s_currKeys[sc]!=0 && s_prevKeys[sc]==0; }
bool Input::keyReleased(SDL_Scancode sc){ return s_currKeys && s_currKeys[sc]==0 && s_prevKeys[sc]!=0; }
} // namespace Erlik