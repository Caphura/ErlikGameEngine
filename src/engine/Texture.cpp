#include "Texture.h"
#include <SDL_image.h>
#include <cstdio>

namespace Erlik {
Texture::~Texture(){ destroy(); }

Texture::Texture(Texture&& o) noexcept { m_tex = o.m_tex; m_w = o.m_w; m_h = o.m_h; o.m_tex = nullptr; o.m_w = o.m_h = 0; }
Texture & Texture::operator=(Texture && o) noexcept {
    if (this != &o) { destroy(); m_tex = o.m_tex; m_w = o.m_w; m_h = o.m_h; o.m_tex = nullptr; o.m_w = o.m_h = 0; }
    return *this;
}

bool Texture::loadFromFile(SDL_Renderer* r, const std::string& path){
    destroy();
    SDL_Surface* surf = IMG_Load(path.c_str());
    if(!surf){ std::fprintf(stderr, "IMG_Load failed: %s\n", IMG_GetError()); return false; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if(!tex){ std::fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError()); SDL_FreeSurface(surf); return false; }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    m_tex = tex; m_w = surf->w; m_h = surf->h; SDL_FreeSurface(surf); return true;
}

bool Texture::createRenderTarget(SDL_Renderer* r, int w, int h, Uint32 fmt) {
    destroy();
    SDL_Texture * tex = SDL_CreateTexture(r, fmt, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex) { std::fprintf(stderr, "CreateTexture(TARGET) failed: %s\n", SDL_GetError()); return false; }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    m_tex = tex; m_w = w; m_h = h; return true;
}

void Texture::destroy(){ if(m_tex){ SDL_DestroyTexture(m_tex); m_tex=nullptr; } m_w=m_h=0; }
} // namespace Erlik