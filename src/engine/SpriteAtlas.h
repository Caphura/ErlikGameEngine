#pragma once
#include <vector>
#include <string>
#include "Texture.h"
struct SDL_Renderer;

namespace Erlik {
class SpriteAtlas{
public:
    bool loadGrid(SDL_Renderer*, const std::string& path, int frameW,int frameH,int margin=0,int spacing=0);
    const Texture& texture() const { return m_tex; }
    const SDL_Rect* frame(int index) const;
    int frameCount() const { return (int)m_frames.size(); }
private:
    Texture m_tex; std::vector<SDL_Rect> m_frames;
};
} // namespace Erlik