#include "Tilemap.h"
#include <fstream>
#include <sstream>
#include <SDL.h>

namespace Erlik {

bool Tilemap::loadCSV(const std::string& path){
    std::ifstream in(path); if(!in) return false;
    m_data.clear();
    std::string line;
    int cols=-1; int rows=0;
    while(std::getline(in, line)){
        if(line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        int c=0;
        while(std::getline(ss, cell, ',')){
            int v = std::stoi(cell);
            m_data.push_back(v);
            c++;
        }
        if(cols<0) cols=c;
        rows++;
    }
    m_cols = cols; m_rows = rows;
    return (m_cols>0 && m_rows>0);
}

bool Tilemap::loadTileset(SDL_Renderer* r, const std::string& path, int tileSize, int margin, int spacing){
    m_tile = tileSize; m_margin = margin; m_spacing = spacing;
    return m_tileset.loadFromFile(r, path);
}

void Tilemap::draw(Renderer2D& r2d) const{
    if(!m_tileset.sdl() || m_cols<=0 || m_rows<=0) return;

    // Visible region (culling)
    int vw, vh; r2d.outputSize(vw, vh);
    Camera2D cam = r2d.camera();
    float left = cam.x, top = cam.y;
    float right = cam.x + vw / cam.zoom, bottom = cam.y + vh / cam.zoom;

    int tx0 = (int)std::floor(left / m_tile);
    int ty0 = (int)std::floor(top  / m_tile);
    int tx1 = (int)std::floor((right  - 1) / m_tile);
    int ty1 = (int)std::floor((bottom - 1) / m_tile);

    if(tx0<0) tx0=0; if(ty0<0) ty0=0;
    if(tx1>=m_cols) tx1=m_cols-1; if(ty1>=m_rows) ty1=m_rows-1;

    // Tileset layout
    int tilesPerRow = m_tileset.width() / m_tile;

    for(int ty=ty0; ty<=ty1; ++ty){
        for(int tx=tx0; tx<=tx1; ++tx){
            int idx = get(tx,ty);
            if(idx < 0) continue;
            int sx = (idx % tilesPerRow) * m_tile;
            int sy = (idx / tilesPerRow) * m_tile;
            SDL_Rect src{ sx, sy, m_tile, m_tile };
            float cx = tx * m_tile + m_tile*0.5f;
            float cy = ty * m_tile + m_tile*0.5f;
            r2d.drawTextureRegion(m_tileset, src, cx, cy, 1.0f, 0.0f);
        }
    }
}

} // namespace Erlik