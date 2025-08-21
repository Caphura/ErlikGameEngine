#pragma once
#include <SDL.h>
#include <string>

namespace Erlik {

class Texture {
public:
    Texture() = default;
    ~Texture();
    bool loadFromFile(SDL_Renderer* renderer, const std::string& path);
    void destroy();
    SDL_Texture* sdl() const { return m_tex; }
    int width() const { return m_w; }
    int height() const { return m_h; }
private:
    SDL_Texture* m_tex = nullptr;
    int m_w = 0, m_h = 0;
};

} // namespace Erlik