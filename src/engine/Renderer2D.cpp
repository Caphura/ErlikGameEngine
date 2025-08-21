#include "Renderer2D.h"
#include <SDL.h>
#include <cmath>

namespace Erlik {

void Renderer2D::clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(m_r, r, g, b, a);
    SDL_RenderClear(m_r);
}

void Renderer2D::present() {
    SDL_RenderPresent(m_r);
}

void Renderer2D::drawTexture(const Texture& tex, float cx, float cy, float scale, float rotationDeg) {
    if (!tex.sdl()) return;
    int w = (int)(tex.width() * scale * m_cam.zoom);
    int h = (int)(tex.height() * scale * m_cam.zoom);
    SDL_FRect dst{ (cx - m_cam.x)*m_cam.zoom - w*0.5f, (cy - m_cam.y)*m_cam.zoom - h*0.5f, (float)w, (float)h };
    SDL_FPoint center{ dst.w*0.5f, dst.h*0.5f };
    SDL_RenderCopyExF(m_r, tex.sdl(), nullptr, &dst, rotationDeg, &center, SDL_FLIP_NONE);
}

void Renderer2D::drawTextureRegion(const Texture& tex, const SDL_Rect& src, float cx, float cy, float scale, float rotationDeg) {
    if (!tex.sdl()) return;
    int w = (int)(src.w * scale * m_cam.zoom);
    int h = (int)(src.h * scale * m_cam.zoom);
    SDL_FRect dst{ (cx - m_cam.x)*m_cam.zoom - w*0.5f, (cy - m_cam.y)*m_cam.zoom - h*0.5f, (float)w, (float)h };
    SDL_FPoint center{ dst.w*0.5f, dst.h*0.5f };
    SDL_RenderCopyExF(m_r, tex.sdl(), &src, &dst, rotationDeg, &center, SDL_FLIP_NONE);
}

void Renderer2D::drawGrid(int spacing, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (spacing <= 0) return;
    SDL_SetRenderDrawColor(m_r, r, g, b, a);
    int w, h; SDL_GetRendererOutputSize(m_r, &w, &h);
    const float left   = m_cam.x;
    const float top    = m_cam.y;
    const float right  = m_cam.x + w / m_cam.zoom;
    const float bottom = m_cam.y + h / m_cam.zoom;
    const int startX = (int)std::floor(left  / spacing) * spacing;
    const int startY = (int)std::floor(top   / spacing) * spacing;
    const int maxX = (int)std::ceil(right);
    const int maxY = (int)std::ceil(bottom);
    for (int x = startX; x <= maxX; x += spacing) {
        float sx = (x - m_cam.x) * m_cam.zoom;
        SDL_RenderDrawLineF(m_r, sx, 0.0f, sx, (float)h);
    }
    for (int y = startY; y <= maxY; y += spacing) {
        float sy = (y - m_cam.y) * m_cam.zoom;
        SDL_RenderDrawLineF(m_r, 0.0f, sy, (float)w, sy);
    }
}

void Renderer2D::outputSize(int& w, int& h) const {
    SDL_GetRendererOutputSize(m_r, &w, &h);
}

} // namespace Erlik