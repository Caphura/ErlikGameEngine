#include "Texture.h"
#include <SDL_image.h>
#include <cstdio>

namespace Erlik {
Texture::~Texture(){ destroy(); }
bool Texture::loadFromFile(SDL_Renderer* r, const std::string& path){
    destroy();
    SDL_Surface* surf = IMG_Load(path.c_str());
    if(!surf){ std::fprintf(stderr, "IMG_Load failed: %s\n", IMG_GetError()); return false; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if(!tex){ std::fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError()); SDL_FreeSurface(surf); return false; }
    m_tex = tex; m_w = surf->w; m_h = surf->h; SDL_FreeSurface(surf); return true;
}
void Texture::destroy(){ if(m_tex){ SDL_DestroyTexture(m_tex); m_tex=nullptr; } m_w=m_h=0; }
} // namespace Erlik