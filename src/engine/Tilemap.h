#pragma once
#include <vector>
#include <string>
#include "Texture.h"
#include "Renderer2D.h"

namespace Erlik {

class Tilemap {
public:
    // CSV: rows separated by '\n', columns by ',' ; -1 = empty
    bool loadCSV(const std::string& path);
    bool loadTileset(SDL_Renderer* r, const std::string& path, int tileSize, int margin=0, int spacing=0);


    void draw(Renderer2D& r2d) const;

    // Queries
    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    int tileSize() const { return m_tile; }
    int get(int tx, int ty) const { if(tx<0||ty<0||tx>=m_cols||ty>=m_rows) return -1; return m_data[ty*m_cols + tx]; }
    // Tilemap.h (class Tilemap içinde, public:)
    bool isSolid(int idx)   const { return idx == 0 || idx >= 2; } // 0 ve 2..N: tam blok
    bool isOneWay(int idx)  const { return idx == 1; }             // 1: tek yönlü
    bool solidAtTile(int tx, int ty) const { return isSolid(get(tx, ty)); }
    bool oneWayAtTile(int tx, int ty) const { return isOneWay(get(tx, ty)); }



private:
    Texture m_tileset;
    int m_tile = 32;
    int m_margin = 0, m_spacing = 0;
    int m_cols = 0, m_rows = 0;
    std::vector<int> m_data; // row-major
};

} // namespace Erlik