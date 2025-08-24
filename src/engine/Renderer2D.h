#pragma once
#include <SDL.h>
#include "Texture.h"

namespace Erlik {

struct Camera2D { float x=0.f; float y=0.f; float zoom=1.f; };

class Renderer2D {
public:
    explicit Renderer2D(SDL_Renderer* r): m_r(r) {}
    void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a=255);
    void present();
    void outputSize(int& w, int& h) const;
    void fillRect(float x, float y, float w, float h, SDL_Color color);
    void drawTexture(const Texture&, float cx,float cy,float scale=1.f,float rotationDeg=0.f);
    void beginFrame() { m_drawCalls = 0; }
    int  drawCalls() const { return m_drawCalls; }

    // imza SONUNA varsayýlan flip eklendi
    void drawTextureRegion(const Texture&, const SDL_Rect& src,
        float cx, float cy, float scale = 1.f, float rotationDeg = 0.f,
        SDL_RendererFlip flip = SDL_FLIP_NONE);

    void drawGrid(int spacing=64, Uint8 r=40,Uint8 g=40,Uint8 b=48,Uint8 a=255);

    void setCamera(const Camera2D& c){ m_cam=c; }
    const Camera2D& camera() const { return m_cam; }

private:
    SDL_Renderer* m_r = nullptr;
    Camera2D m_cam;
    int  m_drawCalls = 0;
};

} // namespace Erlik