#include "Tilemap.h"
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdio>

namespace Erlik {

bool Tilemap::loadCSV(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::fprintf(stderr, "Tilemap CSV open failed: %s\n", path.c_str());
        return false;
    }
    m_data.clear();
    m_cols = 0;
    m_rows = 0;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        int colCount = 0;
        while (std::getline(ss, cell, ',')) {
            // trim whitespace
            size_t start = cell.find_first_not_of(" \t\r\n");
            size_t end   = cell.find_last_not_of(" \t\r\n");
            std::string trimmed = (start==std::string::npos) ? "" : cell.substr(start, end - start + 1);
            if (trimmed.empty()) {
                m_data.push_back(-1);
            } else {
                try {
                    int v = std::stoi(trimmed);
                    m_data.push_back(v);
                } catch (...) {
                    m_data.push_back(-1);
                }
            }
            colCount++;
        }
        if (m_cols == 0) m_cols = colCount;
        m_rows++;
    }
    return (m_cols > 0 && m_rows > 0 && (int)m_data.size() == m_cols * m_rows);
}

bool Tilemap::loadTileset(SDL_Renderer* renderer, const std::string& path, int tileW, int tileH, int margin, int spacing) {
    if (!m_tileset.loadFromFile(renderer, path)) return false;
    m_tileW = tileW; m_tileH = tileH; m_margin = margin; m_spacing = spacing;
    return true;
}

bool Tilemap::tileSrcRect(int index, SDL_Rect& out) const {
    if (index < 0) return false;
    if (!m_tileset.sdl()) return false;

    int texW = m_tileset.width();
    int texH = m_tileset.height();

    int usableW = texW - 2 * m_margin;
    int tilesPerRow = (usableW + m_spacing) / (m_tileW + m_spacing);
    if (tilesPerRow <= 0) return false;

    int xIndex = index % tilesPerRow;
    int yIndex = index / tilesPerRow;

    int x = m_margin + xIndex * (m_tileW + m_spacing);
    int y = m_margin + yIndex * (m_tileH + m_spacing);
    if (x + m_tileW > texW || y + m_tileH > texH) return false;

    out.x = x; out.y = y; out.w = m_tileW; out.h = m_tileH;
    return true;
}

void Tilemap::draw(Renderer2D& r2d) const {
    if (!m_tileset.sdl() || m_cols == 0 || m_rows == 0) return;

    // Görünür alanı hesapla
    int screenW, screenH; r2d.outputSize(screenW, screenH);
    const Camera2D& cam = r2d.camera();
    float left   = cam.x;
    float top    = cam.y;
    float right  = cam.x + screenW / cam.zoom;
    float bottom = cam.y + screenH / cam.zoom;

    int c0 = (int)std::floor(left   / m_tileW) - 1;
    int c1 = (int)std::floor(right  / m_tileW) + 1;
    int r0 = (int)std::floor(top    / m_tileH) - 1;
    int r1 = (int)std::floor(bottom / m_tileH) + 1;

    if (c0 < 0) c0 = 0;
    if (r0 < 0) r0 = 0;
    if (c1 >= m_cols) c1 = m_cols - 1;
    if (r1 >= m_rows) r1 = m_rows - 1;

    SDL_Rect src;
    for (int r = r0; r <= r1; ++r) {
        for (int c = c0; c <= c1; ++c) {
            int idx = tileAt(c, r);
            if (idx < 0) continue;
            if (!tileSrcRect(idx, src)) continue;

            float cx = c * (float)m_tileW + m_tileW * 0.5f;
            float cy = r * (float)m_tileH + m_tileH * 0.5f;
            r2d.drawTextureRegion(m_tileset, src, cx, cy, 1.0f, 0.0f);
        }
    }
}

} // namespace Erlik