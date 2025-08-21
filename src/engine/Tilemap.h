#pragma once
#include <vector>
#include <string>
#include "Texture.h"
#include "Renderer2D.h"

namespace Erlik {

class Tilemap {
public:
    // CSV içeriğini satır-satır okuyup m_cols/m_rows ve m_data'yı doldurur.
    bool loadCSV(const std::string& path);

    // Tileset PNG'ini yükle ve ızgara parametrelerini ayarla.
    bool loadTileset(SDL_Renderer* renderer, const std::string& path, int tileW, int tileH, int margin=0, int spacing=0);

    // Tilemap'i kameraya göre ekrana çizer (culling yapar).
    void draw(Renderer2D& r2d) const;

    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    int tileW() const { return m_tileW; }
    int tileH() const { return m_tileH; }

private:
    bool tileSrcRect(int index, SDL_Rect& out) const;
    int tileAt(int c, int r) const {
        if (c < 0 || c >= m_cols || r < 0 || r >= m_rows) return -1;
        return m_data[r * m_cols + c];
    }

private:
    int m_cols = 0;
    int m_rows = 0;
    std::vector<int> m_data; // -1 boş, 0..N-1 frame index

    Texture m_tileset;
    int m_tileW = 0;
    int m_tileH = 0;
    int m_margin = 0;
    int m_spacing = 0;
};

} // namespace Erlik