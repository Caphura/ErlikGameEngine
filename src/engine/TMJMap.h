#pragma once
#include <string>
#include <vector>
#include "Texture.h"
#include "Renderer2D.h"

namespace Erlik {

    struct Trigger {
        int         id = 0;   // Tiled object id
        std::string type;       // "checkpoint" | "door" | "region" (lowercase)
        std::string name;       // obje adý (ops.)
        std::string target;     // properties.target (ops.)
        std::string message;    // properties.message (ops.)
        bool        once = false; // properties.once
        float       x = 0, y = 0, w = 0, h = 0; // world-space rect (top-left + size)
    };

    class TMJMap {
    public:
        bool load(SDL_Renderer* r, const std::string& tmjPath);
        void draw(Renderer2D& r2d) const;             // hepsini çizer (debug)
        void drawBelowPlayer(Renderer2D& r2d) const;  // fg=false olanlarý çizer
        void drawAbovePlayer(Renderer2D& r2d) const;  // fg=true olanlarý çizer
        const std::vector<Trigger>& triggers() const { return m_triggers; }
        void drawTriggersDebug(class Renderer2D& r2d) const;


        // Fizik için collision grid üret (Tilemap’e doldurur)
        // collisionLayerName="collision", oneWayLayerName="oneway"
        bool buildCollision(class Tilemap& out,
            const std::string& collisionLayerName = "collision",
            const std::string& oneWayLayerName = "oneway") const;

        // Bilgiler
        int cols() const { return m_mapCols; }
        int rows() const { return m_mapRows; }
        int tileW() const { return m_tileW; }
        int tileH() const { return m_tileH; }

    private:
        struct Layer {
            std::string name;
            std::vector<uint32_t> data; // gid (flip bayraklarý maskelenmemiþ ham gid)
            bool   visible = true;
            float  opacity = 1.0f;   // 0..1
            float  parallaxX = 1.0f; // Tiled alaný
            float  parallaxY = 1.0f;
            float  offsetX = 0.0f; // layer offset
            float  offsetY = 0.0f;

            // YENÝ: Tiled Layer Properties
            bool propCollision = false;
            bool propOneWay = false;
            bool propFG = false;   // <-- FG katmaný mý?
        };

        // Tiled gid flip bayraklarý
        static constexpr uint32_t FLIP_H = 0x80000000u;
        static constexpr uint32_t FLIP_V = 0x40000000u;
        static constexpr uint32_t FLIP_D = 0x20000000u;
        static constexpr uint32_t GID_MASK = ~(FLIP_H | FLIP_V | FLIP_D);

        // Tileset bilgisi (tek tileset varsayýyoruz)
        Texture m_tileset;
        int  m_tileW = 32, m_tileH = 32;
        int  m_columns = 0;
        int  m_margin = 0, m_spacing = 0;
        uint32_t m_firstGid = 1;

        int  m_mapCols = 0, m_mapRows = 0;

        std::string m_baseDir; // resim yolu çözmek için
        std::vector<Layer> m_layers;
        std::vector<Trigger> m_triggers;

        // Yardýmcýlar
        static std::string dirOf(const std::string& path);
    };

} // namespace Erlik
