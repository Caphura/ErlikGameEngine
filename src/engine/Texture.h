#pragma once
#include <SDL.h>
#include <string>

namespace Erlik {

class Texture {
public:
    Texture() = default;
    ~Texture();
    Texture(const Texture&) = delete;
    Texture & operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture & operator=(Texture&&) noexcept;
    bool loadFromFile(SDL_Renderer* renderer, const std::string& path);
    // Render target (offscreen) oluþtur
    bool createRenderTarget(SDL_Renderer * renderer, int w, int h,
        Uint32 fmt = SDL_PIXELFORMAT_RGBA8888);
    void destroy();
    SDL_Texture* sdl() const { return m_tex; }
    int width() const { return m_w; }
    int height() const { return m_h; }
    // Küçük yardýmcýlar
    void setAlpha(Uint8 a) const { if (m_tex) SDL_SetTextureAlphaMod(m_tex, a); }
    void setBlend(SDL_BlendMode m) const { if (m_tex) SDL_SetTextureBlendMode(m_tex, m); }
private:
    SDL_Texture* m_tex = nullptr;
    int m_w = 0, m_h = 0;
};

} // namespace Erlik