#include "TMJMap.h"
#include "Tilemap.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <algorithm>      // std::clamp için (iyi olur)
#include <SDL.h>

using nlohmann::json;

namespace Erlik {

    std::string TMJMap::dirOf(const std::string& p) {
        size_t pos = p.find_last_of("/\\");
        return (pos == std::string::npos) ? std::string() : p.substr(0, pos + 1);
    }

    bool TMJMap::load(SDL_Renderer* r, const std::string& tmjPath) {
        std::ifstream in(tmjPath);
        if (!in) return false;
        json j; in >> j;

        m_baseDir = dirOf(tmjPath);

        // Map boyutlarý
        m_mapCols = j.value("width", 0);
        m_mapRows = j.value("height", 0);
        m_tileW = j.value("tilewidth", 32);
        m_tileH = j.value("tileheight", 32);

        // Tileset (tek tileset ve image alanýný bekliyoruz)
        // Tileset (tek tileset)
        auto tilesets = j["tilesets"];
        if (!tilesets.is_array() || tilesets.empty()) return false;
        const auto& ts = tilesets[0];
        m_firstGid = ts.value("firstgid", 1u);
        m_columns = ts.value("columns", 0);
        m_margin = ts.value("margin", 0);
        m_spacing = ts.value("spacing", 0);

        // ---- YOL DÜZELTME: önce olduðu gibi dene, sonra baseDir ile tekrar dene
        std::string image = ts.value("image", "");
        std::string try1 = image;
        std::string try2 = m_baseDir.empty() ? image : (m_baseDir + image);

        bool ok = m_tileset.loadFromFile(r, try1);
        if (!ok && try2 != try1) {
            ok = m_tileset.loadFromFile(r, try2);
            if (ok) SDL_Log("TMJMap: tileset loaded via baseDir: %s", try2.c_str());
        }
        if (!ok) {
            SDL_Log("TMJMap: tileset yuklenemedi (tried \"%s\" and \"%s\")", try1.c_str(), try2.c_str());
            return false;
        }


        // Katmanlar
        m_layers.clear();
        for (const auto& lj : j["layers"]) {
            if (lj.value("type", std::string()) != "tilelayer") continue;

            Layer L;
            L.name = lj.value("name", std::string());
            L.visible = lj.value("visible", true);
            L.opacity = (float)lj.value("opacity", 1.0);
            L.offsetX = (float)lj.value("offsetx", 0.0);
            L.offsetY = (float)lj.value("offsety", 0.0);
            // Tiled 1.10+: parallaxx/parallaxy alanlarý olabilir
            if (lj.contains("parallaxx")) L.parallaxX = (float)lj["parallaxx"].get<double>();
            if (lj.contains("parallaxy")) L.parallaxY = (float)lj["parallaxy"].get<double>();

            // Data
            const auto& arr = lj["data"];
            L.data.resize(arr.size());
            for (size_t i = 0; i < arr.size(); ++i) L.data[i] = arr[i].get<uint32_t>();

            m_layers.push_back(std::move(L));
        }

        return (m_mapCols > 0 && m_mapRows > 0 && !m_layers.empty());
    }

    void TMJMap::draw(Renderer2D& r2d) const {
        if (!m_tileset.sdl()) return;

        // Ana kamera
        const Camera2D base = r2d.camera();
        int vw, vh; r2d.outputSize(vw, vh);

        // Tileset sütun sayýsý
        int tilesPerRow = (m_columns > 0) ? m_columns : (m_tileset.width() / m_tileW);

        // Her katman için
        for (const auto& L : m_layers) {
            if (!L.visible || L.opacity <= 0.f) continue;

            // Parallax için katmana özel kamera
            Camera2D cam = base;
            cam.x = base.x * L.parallaxX - L.offsetX;
            cam.y = base.y * L.parallaxY - L.offsetY;
            r2d.setCamera(cam);

            // Culling
            float left = cam.x, top = cam.y;
            float right = cam.x + vw / cam.zoom, bottom = cam.y + vh / cam.zoom;
            int tx0 = (int)std::floor(left / m_tileW);
            int ty0 = (int)std::floor(top / m_tileH);
            int tx1 = (int)std::floor((right - 1) / m_tileW);
            int ty1 = (int)std::floor((bottom - 1) / m_tileH);
            if (tx0 < 0) tx0 = 0; if (ty0 < 0) ty0 = 0;
            if (tx1 >= m_mapCols) tx1 = m_mapCols - 1; if (ty1 >= m_mapRows) ty1 = m_mapRows - 1;

            // Opacity (alpha mod)
            Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
            SDL_SetTextureAlphaMod(m_tileset.sdl(), alpha);

            for (int ty = ty0; ty <= ty1; ++ty) {
                for (int tx = tx0; tx <= tx1; ++tx) {
                    size_t idx = (size_t)ty * (size_t)m_mapCols + (size_t)tx;
                    uint32_t gidRaw = (idx < L.data.size()) ? L.data[idx] : 0u;
                    if (gidRaw == 0u) continue;

                    // Flip bayraklarý ayrýþtýr
                    uint32_t gid = gidRaw & GID_MASK;
                    const bool flipH = (gidRaw & FLIP_H) != 0;
                    const bool flipV = (gidRaw & FLIP_V) != 0;
                    // Diagonal flip'i þimdilik görmezden geliyoruz

                    int local = (int)gid - (int)m_firstGid; // tileset içi index
                    if (local < 0) continue;

                    int sx = m_margin + (local % tilesPerRow) * (m_tileW + m_spacing);
                    int sy = m_margin + (local / tilesPerRow) * (m_tileH + m_spacing);

                    SDL_Rect src{ sx, sy, m_tileW, m_tileH };

                    float cx = tx * (float)m_tileW + m_tileW * 0.5f + L.offsetX;
                    float cy = ty * (float)m_tileH + m_tileH * 0.5f + L.offsetY;

                    SDL_RendererFlip flip = SDL_FLIP_NONE;
                    if (flipH) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
                    if (flipV) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

                    r2d.drawTextureRegion(m_tileset, src, cx, cy, 1.0f, 0.0f, flip);
                }
            }

            // Alpha modunu sýfýrla
            SDL_SetTextureAlphaMod(m_tileset.sdl(), 255);
        }

        // Ana kamerayý geri koy
        r2d.setCamera(base);
    }

    bool TMJMap::buildCollision(Tilemap& out,
        const std::string& collisionLayerName,
        const std::string& oneWayLayerName) const
    {
        if (m_mapCols <= 0 || m_mapRows <= 0) return false;
        std::vector<int> grid(m_mapCols * m_mapRows, -1);

        auto applyLayer = [&](const std::string& name, int value) {
            for (const auto& L : m_layers) {
                if (L.name != name) continue;
                for (size_t i = 0; i < L.data.size() && i < grid.size(); ++i) {
                    uint32_t gid = (L.data[i] & GID_MASK);
                    if (gid != 0u) {
                        grid[i] = value; // 0 = solid, 1 = oneway
                    }
                }
            }
        };

        applyLayer(collisionLayerName, 0);
        applyLayer(oneWayLayerName, 1);

        return out.adoptGrid(m_mapCols, m_mapRows, m_tileW, std::move(grid));
    }

} // namespace Erlik
