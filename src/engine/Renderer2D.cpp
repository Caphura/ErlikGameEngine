#include "Renderer2D.h"
#include <SDL.h>
#include <cmath>

namespace Erlik{
void Renderer2D::clear(Uint8 r,Uint8 g,Uint8 b,Uint8 a){ SDL_SetRenderDrawColor(m_r,r,g,b,a); SDL_RenderClear(m_r); }
void Renderer2D::present(){ SDL_RenderPresent(m_r); }
// Renderer2D.cpp
void Renderer2D::drawGrid(int spacing, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (spacing <= 0) return;
    SDL_SetRenderDrawColor(m_r, r, g, b, a);

    int w, h; SDL_GetRendererOutputSize(m_r, &w, &h);

    // Ekran->dünya sýnýrlarý
        const float left = m_cam.x;
    const float top = m_cam.y;
    const float right = m_cam.x + w / m_cam.zoom;
    const float bottom = m_cam.y + h / m_cam.zoom;

    // Izgarayý hücre baþlangýcýna hizala
    const int startX = static_cast<int>(std::floor(left / spacing)) * spacing;
    const int startY = static_cast<int>(std::floor(top / spacing)) * spacing;

    // Üst sýnýrý tamsayýya sabitle (opsiyonel ama temiz)
    const int maxX = static_cast<int>(std::ceil(right));
    const int maxY = static_cast<int>(std::ceil(bottom));

    // Dikey çizgiler
    for (int x = startX; x <= maxX; x += spacing) {
        float sx = (x - m_cam.x) * m_cam.zoom;
        SDL_RenderDrawLineF(m_r, sx, 0.0f, sx, static_cast<float>(h));
    }
    // Yatay çizgiler
    for (int y = startY; y <= maxY; y += spacing) {
        float sy = (y - m_cam.y) * m_cam.zoom;
        SDL_RenderDrawLineF(m_r, 0.0f, sy, static_cast<float>(w), sy);
    }
}

void Renderer2D::drawTexture(const Texture& tex,float cx,float cy,float scale,float rot){
 if(!tex.sdl()) return;
 int w=(int)(tex.width()*scale*m_cam.zoom), h=(int)(tex.height()*scale*m_cam.zoom);
 SDL_FRect dst{ (cx - m_cam.x)*m_cam.zoom - w*0.5f, (cy - m_cam.y)*m_cam.zoom - h*0.5f, (float)w, (float)h };
 SDL_FPoint center{ dst.w*0.5f, dst.h*0.5f };
 SDL_RenderCopyExF(m_r, tex.sdl(), nullptr, &dst, rot, &center, SDL_FLIP_NONE);
} }
