#include "TMJMap.h"
#include "Tilemap.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <SDL.h>
#include <cctype>

using nlohmann::json;

namespace Erlik {

    static std::string tolower_copy(std::string s) {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    }

    static bool json_bool_or(const nlohmann::json& o, const char* name, bool defv) {
        if (!o.contains("properties")) return defv;
        for (auto& p : o["properties"]) {
            if (p.value("name", "") == name) {
                if (p.contains("value") && p["value"].is_boolean()) return p["value"].get<bool>();
            }
        }
        return defv;
    }

    static std::string json_str_or(const nlohmann::json& o, const char* name, const std::string& defv) {
        if (!o.contains("properties")) return defv;
        for (auto& p : o["properties"]) {
            if (p.value("name", "") == name) {
                if (p.contains("value")) {
                    if (p["value"].is_string())  return p["value"].get<std::string>();
                    if (p["value"].is_number())  return p["value"].dump(); // sayýyý stringle
                }
            }
        }
        return defv;
    }

    static float json_float_or(const nlohmann::json& o, const char* name, float defv) {
        if (!o.contains("properties")) return defv;
        for (auto& p : o["properties"]) {
            if (p.value("name", "") == name) {
                if (p.contains("value")) {
                    if (p["value"].is_number()) return p["value"].get<float>();
                    if (p["value"].is_string()) { try { return std::stof(p["value"].get<std::string>()); } catch (...) {} }
                }
            }
        }
        return defv;
    }


    std::string TMJMap::dirOf(const std::string& p) {
        size_t pos = p.find_last_of("/\\");
        return (pos == std::string::npos) ? std::string() : p.substr(0, pos + 1);
    }

    static std::string readFileText(const std::string& p) {
        std::ifstream f(p, std::ios::binary); if (!f) return {};
        std::string s; f.seekg(0, std::ios::end); s.resize((size_t)f.tellg());
        f.seekg(0, std::ios::beg); f.read(&s[0], (std::streamsize)s.size()); return s;
    }
    static std::string getAttr(const std::string& tag, const std::string& name) {
        auto p = tag.find(name + "="); if (p == std::string::npos) return {};
        p += name.size() + 1; if (p >= tag.size()) return {};
        char q = tag[p]; if (q != '\"' && q != '\'') return {};
        auto s = p + 1; auto e = tag.find(q, s); if (e == std::string::npos) return {};
        return tag.substr(s, e - s);
    }
    static int getAttrInt(const std::string& tag, const std::string& name, int def = 0) {
        auto v = getAttr(tag, name); if (v.empty()) return def; try { return std::stoi(v); }
        catch (...) { return def; }
    }

    bool TMJMap::load(SDL_Renderer* r, const std::string& tmjPath)
    {
        // --- TMJ dosyasýný oku ---
        std::ifstream in(tmjPath);
        if (!in) {
            SDL_Log("TMJMap: tmj not found: %s", tmjPath.c_str());
            return false;
        }
        json j; in >> j;

        m_baseDir = dirOf(tmjPath);
        // Önceki yüklemeden kalmýþ cache’leri temizle (hot-reload için)
        destroyCaches();

        // Harita boyutlarý
        m_mapCols = j.value("width", 0);
        m_mapRows = j.value("height", 0);
        m_tileW = j.value("tilewidth", 32);
        m_tileH = j.value("tileheight", 32);

        // --- Tileset (tek tileset bekliyoruz) ---
        auto tilesets = j["tilesets"];
        if (!tilesets.is_array() || tilesets.empty()) return false;
        const auto& ts = tilesets[0];
        m_firstGid = ts.value("firstgid", 1u);

        std::string image;
        m_columns = ts.value("columns", 0);
        m_margin = ts.value("margin", 0);
        m_spacing = ts.value("spacing", 0);

        if (ts.contains("image")) {
            // INLINE TILESET (TMJ içinde image alaný var)
            image = ts.value("image", "");
            // (Varsa TMJ'deki tilewidth/height deðerleriyle override edilebilir)
            m_tileW = j.value("tilewidth", m_tileW);
            m_tileH = j.value("tileheight", m_tileH);
        }
        else if (ts.contains("source")) {
            // EXTERNAL .TSX
            std::string tsxRel = ts.value("source", "");
            std::string tsxPath = m_baseDir.empty() ? tsxRel : (m_baseDir + tsxRel);
            std::string xml = readFileText(tsxPath);
            if (xml.empty()) { SDL_Log("TMJMap: tsx not found: %s", tsxPath.c_str()); return false; }

            // <tileset ...>
            auto p0 = xml.find("<tileset"); if (p0 == std::string::npos) { SDL_Log("TMJMap: tsx invalid"); return false; }
            auto p1 = xml.find('>', p0);    if (p1 == std::string::npos) { SDL_Log("TMJMap: tsx invalid"); return false; }
            std::string tstag = xml.substr(p0, p1 - p0 + 1);

            // <image .../>
            auto i0 = xml.find("<image"); if (i0 == std::string::npos) { SDL_Log("TMJMap: tsx missing <image>"); return false; }
            auto i1 = xml.find('>', i0);  if (i1 == std::string::npos) { SDL_Log("TMJMap: tsx invalid <image>"); return false; }
            std::string imgtag = xml.substr(i0, i1 - i0 + 1);

            // Öznitelikler
            int tw = getAttrInt(tstag, "tilewidth", m_tileW);
            int th = getAttrInt(tstag, "tileheight", m_tileH);
            int mar = getAttrInt(tstag, "margin", 0);
            int sp = getAttrInt(tstag, "spacing", 0);
            int cols = getAttrInt(tstag, "columns", 0);

            std::string imgRel = getAttr(imgtag, "source");
            std::string imgPath = imgRel;
            if (!m_baseDir.empty() && !imgRel.empty()) imgPath = m_baseDir + imgRel;

            // Uygula
            m_tileW = tw; m_tileH = th; m_margin = mar; m_spacing = sp; m_columns = cols;
            image = imgPath;

            SDL_Log("TMJMap: tsx parsed (tw=%d th=%d margin=%d spacing=%d columns=%d image=%s)",
                m_tileW, m_tileH, m_margin, m_spacing, m_columns, image.c_str());
        }
        else {
            SDL_Log("TMJMap: tileset has neither image nor source");
            return false;
        }

        // Görseli yükle (iki deneme: olduðu gibi, sonra baseDir ile)
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

        // --- Parse object layer "triggers" ---
        m_triggers.clear();
        if (j.contains("layers") && j["layers"].is_array()) {
            for (const auto& L : j["layers"]) {
                if (L.value("type", "") != "objectgroup") continue;
                if (tolower_copy(L.value("name", "")) != "triggers") continue;
                if (!L.contains("objects") || !L["objects"].is_array()) continue;

                for (const auto& O : L["objects"]) {
                    Trigger t;
                    t.id = O.value("id", 0);
                    t.name = O.value("name", "");
                    t.type = tolower_copy(O.value("type", ""));
                    t.x = O.value("x", 0.0f);
                    t.y = O.value("y", 0.0f);
                    t.w = O.value("width", 0.0f);
                    t.h = O.value("height", 0.0f);
                    t.once = json_bool_or(O, "once", false);
                    t.target = json_str_or(O, "target", "");
                    t.message = json_str_or(O, "message", "");
                    t.zoom = json_float_or(O, "zoom", 0.0f);
                    // door props (opsiyonel)
                    t.sfx = json_str_or(O, "sfx", "");
                    t.fadeInMs = json_float_or(O, "fadeInMs", 0.0f);
                    t.fadeOutMs = json_float_or(O, "fadeOutMs", 0.0f);
                    t.shake = json_float_or(O, "shake", 0.0f);
                    // music region props (opsiyonel)
                    t.music = json_str_or(O, "music", "");
                    t.exitMusic = json_str_or(O, "exitMusic", "");
                    t.musicVol = json_float_or(O, "musicVol", 1.0f);
                    t.musicFadeMs = json_float_or(O, "musicFadeMs", 0.0f);
                    t.musicFadeInMs = json_float_or(O, "musicFadeInMs", 0.0f);
                    t.musicFadeOutMs = json_float_or(O, "musicFadeOutMs", 0.0f);

                    // fallback: sadece musicFadeMs verilmiþse iki tarafa da uygula
                    if (t.musicFadeInMs <= 0.f && t.musicFadeOutMs <= 0.f && t.musicFadeMs > 0.f) {
                        t.musicFadeInMs = t.musicFadeMs;
                        t.musicFadeOutMs = t.musicFadeMs;
                    }


                    if (!t.type.empty() && t.w > 0 && t.h > 0)
                        m_triggers.push_back(std::move(t));
                }
            }
        }
        SDL_Log("INFO: TMJ triggers loaded: %d", (int)m_triggers.size());


        // --- Katmanlarý oku ---
        // --- Katmanlarý oku ---
        m_layers.clear();
        if (j.contains("layers") && j["layers"].is_array()) {
            for (const auto& lj : j["layers"]) {
                if (lj.value("type", std::string()) != "tilelayer") continue;

                Layer L;
                L.name = lj.value("name", std::string());
                L.visible = lj.value("visible", true);
                L.opacity = (float)lj.value("opacity", 1.0);
                L.offsetX = (float)lj.value("offsetx", 0.0);
                L.offsetY = (float)lj.value("offsety", 0.0);
                if (lj.contains("parallaxx")) L.parallaxX = (float)lj["parallaxx"].get<double>();
                if (lj.contains("parallaxy")) L.parallaxY = (float)lj["parallaxy"].get<double>();

                // Data
                const auto& arr = lj["data"];
                L.data.resize(arr.size());
                for (size_t i = 0; i < arr.size(); ++i) L.data[i] = arr[i].get<uint32_t>();

                // (ZATEN VARSA) Layer properties: collision/oneway (bool)
                if (lj.contains("properties") && lj["properties"].is_array()) {
                    for (const auto& pj : lj["properties"]) {
                        std::string pname = pj.value("name", std::string());
                        if (!pj.contains("value")) continue;
                        
                            // --- bool bayraklar ---
                            if (pj["value"].is_boolean()) {
                            bool pval = pj["value"].get<bool>();
                            if (pname == "collision" && pval) L.propCollision = true;
                            if (pname == "oneway" && pval) L.propOneWay = true;
                            if (pname == "fg" && pval) L.propFG = true;
                            if (pname == "static" && pval) L.propStatic = true;
                            continue;
                            
                        }
                        // --- string preset ---
                            if (pj["value"].is_string()) {
                            std::string sval = pj["value"].get<std::string>();
                                                        // normalleþtir
                                for (auto& c : sval) c = (char)std::tolower((unsigned char)c);
                            if (pname == "preset") {
                                L.preset = sval; // uygulamayý birazdan yapacaðýz
                                
                            }
                            continue;
                        }
                            // --- sayýsal override'lar ---
                            if (pj["value"].is_number()) {
                            double d = pj["value"].get<double>();
                            if (pname == "parallaxx") L.parallaxX = (float)d;
                            if (pname == "parallaxy") L.parallaxY = (float)d;
                            if (pname == "offsetx")   L.offsetX = (float)d;
                            if (pname == "offsety")   L.offsetY = (float)d;
                            if (pname == "opacity")   L.opacity = (float)d;
                            continue;
                        }
                    }
                }

                // --- Editor-side Parallax Presets ---
                if (!L.preset.empty()) {
                    const std::string & p = L.preset;
                    // Önerilen isimler:
                    // "bg_sky", "bg_far", "bg_mid", "bg_near", "game", "fg"
                    if (p == "bg_sky") { L.parallaxX = 0.20f; L.parallaxY = 0.10f; /*L.opacity=1.0f;*/ }
                    else if (p == "bg_far") { L.parallaxX = 0.40f; L.parallaxY = 0.30f; }
                    else if (p == "bg_mid") { L.parallaxX = 0.65f; L.parallaxY = 0.60f; }
                    else if (p == "bg_near") { L.parallaxX = 0.85f; L.parallaxY = 0.85f; }
                    else if (p == "game") { L.parallaxX = 1.00f; L.parallaxY = 1.00f; }
                    else if (p == "fg") { L.parallaxX = 1.00f; L.parallaxY = 1.00f; L.propFG = true; }
                    // Ýsteyen layer property ile (parallaxx/parallaxy) preset'in üstünü ezebilir.
                }

                // ÝSÝMDEN otomatik bayraklar (property eklemeyi unutsan da çalýþsýn)
                {
                    std::string lname = L.name;
                    std::transform(lname.begin(), lname.end(), lname.begin(),
                        [](unsigned char c) { return (char)std::tolower(c); });

                    if (lname == "fg" || lname == "foreground")
                        L.propFG = true;

                    if (lname == "collision" || lname == "collisions" || lname == "solid")
                        L.propCollision = true;

                    if (lname == "oneway" || lname == "one-way" || lname == "platforms")
                        L.propOneWay = true;
                }
                m_layers.push_back(std::move(L));
            }
        }

        SDL_Log("TMJMap: map=%dx%d tile=%dx%d layers=%zu firstgid=%u columns=%d",
            m_mapCols, m_mapRows, m_tileW, m_tileH, m_layers.size(),
            (unsigned)m_firstGid, m_columns);

        // Statik cache’leri inþa et (destekliyse)
        buildStaticCaches(r);
        return (m_mapCols > 0 && m_mapRows > 0 && m_tileset.sdl() != nullptr);
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
            cam.x = base.x * L.parallaxX;
            cam.y = base.y * L.parallaxY;

            r2d.setCamera(cam);

      
            // Eðer statik cache varsa tek blit ile çiz ve devam et
            if (L.propStatic && L.cacheTex.sdl()) {
                Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
                L.cacheTex.setAlpha(alpha);
                // Tüm harita boyutunda cache (merkezden çiziyoruz)
                const float mapW = (float)(m_mapCols * m_tileW);
                const float mapH = (float)(m_mapRows * m_tileH);
                const float cx = mapW * 0.5f + L.offsetX;
                const float cy = mapH * 0.5f + L.offsetY;
                r2d.drawTextureSDL(L.cacheTex.sdl(), nullptr, cx, cy, 1.0f, 0.0f, SDL_FLIP_NONE);
                L.cacheTex.setAlpha(255);
                continue;
                
            }

            // Culling cach yoksa normal yol ile yap
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
    void TMJMap::drawBelowPlayer(Renderer2D& r2d) const {
        if (!m_tileset.sdl()) return;
        const Camera2D base = r2d.camera();
        int vw, vh; r2d.outputSize(vw, vh);
        int tilesPerRow = (m_columns > 0) ? m_columns : (m_tileset.width() / m_tileW);

        for (const auto& L : m_layers) {
            if (!L.visible || L.opacity <= 0.f) continue;

            // Fizik katmanlarýný ASLA çizme
            if (L.propCollision || L.propOneWay) continue;

            // Sadece FG OLMAYANLAR burada çizilecek
            if (L.propFG) continue;

            Camera2D cam = base;
            cam.x = base.x * L.parallaxX;
            cam.y = base.y * L.parallaxY;

            r2d.setCamera(cam);

            // Eðer statik cache varsa tek blit ile çiz ve devam et
            if (L.propStatic && L.cacheTex.sdl()) {
                Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
                L.cacheTex.setAlpha(alpha);
                // Tüm harita boyutunda cache (merkezden çiziyoruz)
                const float mapW = (float)(m_mapCols * m_tileW);
                const float mapH = (float)(m_mapRows * m_tileH);
                const float cx = mapW * 0.5f + L.offsetX;
                const float cy = mapH * 0.5f + L.offsetY;
                r2d.drawTextureSDL(L.cacheTex.sdl(), nullptr, cx, cy, 1.0f, 0.0f, SDL_FLIP_NONE);
                L.cacheTex.setAlpha(255);
                continue;

            }

            float left = cam.x, top = cam.y;
            float right = cam.x + vw / cam.zoom, bottom = cam.y + vh / cam.zoom;
            int tx0 = (int)std::floor(left / m_tileW);
            int ty0 = (int)std::floor(top / m_tileH);
            int tx1 = (int)std::floor((right - 1) / m_tileW);
            int ty1 = (int)std::floor((bottom - 1) / m_tileH);
            if (tx0 < 0) tx0 = 0; if (ty0 < 0) ty0 = 0;
            if (tx1 >= m_mapCols) tx1 = m_mapCols - 1; if (ty1 >= m_mapRows) ty1 = m_mapRows - 1;

            Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
            SDL_SetTextureAlphaMod(m_tileset.sdl(), alpha);

            for (int ty = ty0; ty <= ty1; ++ty) {
                for (int tx = tx0; tx <= tx1; ++tx) {
                    size_t idx = (size_t)ty * (size_t)m_mapCols + (size_t)tx;
                    uint32_t gidRaw = (idx < L.data.size()) ? L.data[idx] : 0u;
                    if (gidRaw == 0u) continue;

                    uint32_t gid = gidRaw & GID_MASK;
                    const bool flipH = (gidRaw & FLIP_H) != 0;
                    const bool flipV = (gidRaw & FLIP_V) != 0;

                    int local = (int)gid - (int)m_firstGid;
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
            SDL_SetTextureAlphaMod(m_tileset.sdl(), 255);
        }
        r2d.setCamera(base);
    }

    void TMJMap::drawAbovePlayer(Renderer2D& r2d) const {
        if (!m_tileset.sdl()) return;
        const Camera2D base = r2d.camera();
        int vw, vh; r2d.outputSize(vw, vh);
        int tilesPerRow = (m_columns > 0) ? m_columns : (m_tileset.width() / m_tileW);

        for (const auto& L : m_layers) {
            if (!L.visible || L.opacity <= 0.f) continue;

            // Fizik katmanlarýný ASLA çizme
            if (L.propCollision || L.propOneWay) continue;

            // Sadece FG OLANLAR burada çizilecek
            if (!L.propFG) continue;

            Camera2D cam = base;
            cam.x = base.x * L.parallaxX;
            cam.y = base.y * L.parallaxY;

            r2d.setCamera(cam);

            // Eðer statik cache varsa tek blit ile çiz ve devam et
            if (L.propStatic && L.cacheTex.sdl()) {
                Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
                L.cacheTex.setAlpha(alpha);
                // Tüm harita boyutunda cache (merkezden çiziyoruz)
                const float mapW = (float)(m_mapCols * m_tileW);
                const float mapH = (float)(m_mapRows * m_tileH);
                const float cx = mapW * 0.5f + L.offsetX;
                const float cy = mapH * 0.5f + L.offsetY;
                r2d.drawTextureSDL(L.cacheTex.sdl(), nullptr, cx, cy, 1.0f, 0.0f, SDL_FLIP_NONE);
                L.cacheTex.setAlpha(255);
                continue;

            }

            float left = cam.x, top = cam.y;
            float right = cam.x + vw / cam.zoom, bottom = cam.y + vh / cam.zoom;
            int tx0 = (int)std::floor(left / m_tileW);
            int ty0 = (int)std::floor(top / m_tileH);
            int tx1 = (int)std::floor((right - 1) / m_tileW);
            int ty1 = (int)std::floor((bottom - 1) / m_tileH);
            if (tx0 < 0) tx0 = 0; if (ty0 < 0) ty0 = 0;
            if (tx1 >= m_mapCols) tx1 = m_mapCols - 1; if (ty1 >= m_mapRows) ty1 = m_mapRows - 1;

            Uint8 alpha = (Uint8)std::round(std::clamp(L.opacity, 0.f, 1.f) * 255.f);
            SDL_SetTextureAlphaMod(m_tileset.sdl(), alpha);

            for (int ty = ty0; ty <= ty1; ++ty) {
                for (int tx = tx0; tx <= tx1; ++tx) {
                    size_t idx = (size_t)ty * (size_t)m_mapCols + (size_t)tx;
                    uint32_t gidRaw = (idx < L.data.size()) ? L.data[idx] : 0u;
                    if (gidRaw == 0u) continue;

                    uint32_t gid = gidRaw & GID_MASK;
                    const bool flipH = (gidRaw & FLIP_H) != 0;
                    const bool flipV = (gidRaw & FLIP_V) != 0;

                    int local = (int)gid - (int)m_firstGid;
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
            SDL_SetTextureAlphaMod(m_tileset.sdl(), 255);
        }
        r2d.setCamera(base);
    }


    bool TMJMap::buildCollision(Tilemap& out,
        const std::string& collisionLayerName,
        const std::string& oneWayLayerName) const
    {
        if (m_mapCols <= 0 || m_mapRows <= 0) return false;
        std::vector<int> grid(m_mapCols * m_mapRows, -1);

        int solids = 0, oneways = 0;

        auto applyCollisionLayer = [&](const Layer& L) {
            for (size_t i = 0; i < L.data.size() && i < grid.size(); ++i) {
                uint32_t gid = (L.data[i] & GID_MASK);
                if (gid != 0u) { grid[i] = 0; ++solids; } // 0 = solid
            }
        };
        auto applyOneWayLayer = [&](const Layer& L) {
            for (size_t i = 0; i < L.data.size() && i < grid.size(); ++i) {
                uint32_t gid = (L.data[i] & GID_MASK);
                if (gid != 0u) { grid[i] = 1; ++oneways; } // 1 = oneway
            }
        };

        // 1) Ýsimle eþleþen katmanlarý uygula
        for (const auto& L : m_layers) {
            if (L.name == collisionLayerName) applyCollisionLayer(L);
            if (L.name == oneWayLayerName)    applyOneWayLayer(L);
        }
        // 2) Property’lere göre de uygula (isimden baðýmsýz)
        for (const auto& L : m_layers) {
            if (L.propCollision) applyCollisionLayer(L);
            if (L.propOneWay)    applyOneWayLayer(L);
        }

        SDL_Log("TMJMap: collision build -> solids=%d, oneways=%d (map=%dx%d)",
            solids, oneways, m_mapCols, m_mapRows);

        bool ok = out.adoptGrid(m_mapCols, m_mapRows, m_tileW, std::move(grid));
        return ok && (solids > 0 || oneways > 0);
    }

    

    void TMJMap::drawTriggersDebug(Renderer2D& r2d) const {
        for (auto& t : m_triggers) {
            SDL_Color c{ 180,120,40,90 }; // region
            if (t.type == "checkpoint") c = SDL_Color{ 80,180,80,90 };
            else if (t.type == "door")  c = SDL_Color{ 80,120,200,90 };
            r2d.fillRect(t.x, t.y, t.w, t.h, c);
        }
    }

    const Trigger* TMJMap::findTriggerByName(const std::string& name) const {
        if (name.empty()) return nullptr;
        for (const auto& t : m_triggers) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }

    void TMJMap::destroyCaches() {
        for (auto& L : m_layers) L.cacheTex.destroy();
    }

    bool TMJMap::buildStaticCaches(SDL_Renderer* r) {
        if (!r || !m_tileset.sdl()) return false;

        // Renderer kapasitesi
        SDL_RendererInfo info{}; SDL_GetRendererInfo(r, &info);
        const int maxW = info.max_texture_width ? (int)info.max_texture_width : 16384;
        const int maxH = info.max_texture_height ? (int)info.max_texture_height : 16384;

        const int mapW = m_mapCols * m_tileW;
        const int mapH = m_mapRows * m_tileH;
        if (mapW <= 0 || mapH <= 0) return false;

        const int tilesPerRow = (m_columns > 0) ? m_columns : (m_tileset.width() / m_tileW);

        int built = 0, skipped = 0;

        for (auto& L : m_layers) {
            if (!L.propStatic || !L.visible || L.opacity <= 0.f) { skipped++; continue; }
            if (L.propCollision || L.propOneWay) { skipped++; continue; } // fizik katmanlarý cache’lenmez
            if (mapW > maxW || mapH > maxH) { SDL_Log("static-cache: skipped (too big) %dx%d > limit %dx%d", mapW, mapH, maxW, maxH); skipped++; continue; }

            // Render target oluþtur
            Texture rt;
            if (!rt.createRenderTarget(r, mapW, mapH, SDL_PIXELFORMAT_RGBA8888)) {
                SDL_Log("static-cache: create failed");
                skipped++; continue;
             
            }

            SDL_Texture* prev = SDL_GetRenderTarget(r);
            SDL_SetRenderTarget(r, rt.sdl());
            // Þeffaf temizle
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
            SDL_RenderClear(r);

            // Layer’ý cache’e döþe
            for (int ty = 0; ty < m_mapRows; ++ty) {
                for (int tx = 0; tx < m_mapCols; ++tx) {
                    size_t idx = (size_t)ty * (size_t)m_mapCols + (size_t)tx;
                    uint32_t gidRaw = (idx < L.data.size()) ? L.data[idx] : 0u;
                    if (gidRaw == 0u) continue;

                    uint32_t gid = gidRaw & GID_MASK;
                    const bool flipH = (gidRaw & FLIP_H) != 0;
                    const bool flipV = (gidRaw & FLIP_V) != 0;

                    int local = (int)gid - (int)m_firstGid;
                    if (local < 0) continue;

                    int sx = m_margin + (local % tilesPerRow) * (m_tileW + m_spacing);
                    int sy = m_margin + (local / tilesPerRow) * (m_tileH + m_spacing);
                    SDL_Rect src{ sx, sy, m_tileW, m_tileH };

                    SDL_FRect dst{
                        tx * (float)m_tileW + L.offsetX,
                        ty * (float)m_tileH + L.offsetY,
                        (float)m_tileW, (float)m_tileH
                    };

                    SDL_RendererFlip flip = SDL_FLIP_NONE;
                    if (flipH) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
                    if (flipV) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

                    SDL_RenderCopyExF(r, m_tileset.sdl(), &src, &dst, 0.0, nullptr, flip);
                }
            }

            SDL_SetRenderTarget(r, prev);
            L.cacheTex = std::move(rt); // move-assign
            built++;
        }

        SDL_Log("static-cache: built=%d skipped=%d", built, skipped);
        return built > 0;
    }


} // namespace Erlik
