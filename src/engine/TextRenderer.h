#pragma once
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

namespace Erlik {
    class TextRenderer {
    public:
        bool init(SDL_Renderer* r, const char* fontPath, int pt) {
            if (!TTF_WasInit() && TTF_Init() != 0) return false;
            m_font = TTF_OpenFont(fontPath, pt);
            m_r = r;
            return m_font != nullptr;
        }
        void shutdown() {
            if (m_font) { TTF_CloseFont(m_font); m_font = nullptr; }
            // TTF_Quit()’i burada çaðýrmak istemiyorsan boþ býrak
        }
        void draw(const char* text, int x, int y, SDL_Color col, float scale = 1.f) {
            if (!m_font || !m_r || !text) return;
            SDL_Surface* s = TTF_RenderUTF8_Blended(m_font, text, col);
            if (!s) return;
            SDL_Texture* t = SDL_CreateTextureFromSurface(m_r, s);
            SDL_FreeSurface(s);
            if (!t) return;
            int w, h; SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
            SDL_FRect dst{ (float)x, (float)y, w * scale, h * scale };
            SDL_RenderCopyF(m_r, t, nullptr, &dst);
            SDL_DestroyTexture(t);
        }
    private:
        SDL_Renderer* m_r = nullptr;
        TTF_Font* m_font = nullptr;
    };
} // namespace Erlik
