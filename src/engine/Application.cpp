#include "Application.h"
#include "Input.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>
#include <cmath>      // std::floor
#include <cstring>    // std::strlen, std::strncpy
#include <algorithm>  // std::clamp, std::min, std::max

namespace Erlik {

bool Application::init(){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER)!=0){
        std::fprintf(stderr,"SDL_Init failed: %s\n", SDL_GetError()); return false;
    }
    int flags = IMG_INIT_PNG;
    if((IMG_Init(flags)&flags)!=flags){
        std::fprintf(stderr,"IMG_Init failed: %s\n", IMG_GetError()); return false;
    }
    
    m_window = SDL_CreateWindow("ErlikGameEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_SHOWN);
    if(!m_window){ std::fprintf(stderr,"SDL_CreateWindow failed: %s\n", SDL_GetError()); return false; }
    m_renderer = SDL_CreateRenderer(m_window,-1,SDL_RENDERER_ACCELERATED);
    if(!m_renderer){ std::fprintf(stderr,"SDL_CreateRenderer failed: %s\n", SDL_GetError()); return false; }
    m_r2d = new Renderer2D(m_renderer);
    Input::init();

    // World setup
   // if(!m_map.loadCSV("assets/level_aabb.csv")) std::fprintf(stderr,"[warn] assets/level_aabb.csv not found\n");
    //if(!m_map.loadTileset(m_renderer, "assets/tileset32.png", 32)) std::fprintf(stderr,"[warn] assets/tileset32.png not found\n");
    // TMJ yükle
    // World setup (TMJ)
    m_tmjPath = "assets/level_city.tmj"; // kendi dosya yolun
    if (m_tmj.load(m_renderer, m_tmjPath)) {
        std::fprintf(stderr, "[info] TMJ loaded: %s\n", m_tmjPath.c_str());
        m_tmj.buildCollision(m_map, "collision", "oneway");
        m_worldW = static_cast<float>(m_tmj.cols() * m_tmj.tileW());
        m_worldH = static_cast<float>(m_tmj.rows() * m_tmj.tileH());

        // --- Hot Reload: TMJ dosyasını izle
        m_res.track(m_tmjPath, [this]() {
            if (m_tmj.load(m_renderer, m_tmjPath)) {
                std::fprintf(stderr, "[hotreload] TMJ reloaded: %s\n", m_tmjPath.c_str());
                m_tmj.buildCollision(m_map, "collision", "oneway");
                m_worldW = static_cast<float>(m_tmj.cols() * m_tmj.tileW());
                m_worldH = static_cast<float>(m_tmj.rows() * m_tmj.tileH());

                notifyHUD("Reload OK (TMJ)", SDL_Color{ 40,200, 90,255 }, 1.5f);
            }
            else {
                std::fprintf(stderr, "[hotreload] TMJ reload FAILED: %s\n", m_tmjPath.c_str());
                notifyHUD("Reload FAIL (TMJ)", SDL_Color{ 220, 60, 60,255 }, 2.0f);
            }
            });


    }
    else {
        std::fprintf(stderr, "[warn] TMJ load FAILED, fallback CSV\n");
        m_map.loadCSV("assets/level_aabb.csv");
        m_map.loadTileset(m_renderer, "assets/tileset32.png", 32);
        m_worldW = static_cast<float>(m_map.cols() * m_map.tileSize());
        m_worldH = static_cast<float>(m_map.rows() * m_map.tileSize());
    }

    // renderer kurulumundan sonra:
    if (!m_text.init(m_renderer, m_fontPath, 14)) {
        std::fprintf(stderr, "[dbg] TTF load failed (%s) -> overlay text disabled\n", m_fontPath);
    }

    bool fontOK = m_text.init(m_renderer, m_fontPath, 14);
    if (!fontOK) {
        // Windows klasik fontu + muhtemel asset yollarını dene
        const char* fallbacks[] = {
            "C:/Windows/Fonts/arial.ttf",
            "assets/DejaVuSans.ttf",
            "assets/fonts/DejaVuSans.ttf"
        };
        for (const char* fb : fallbacks) {
            if (m_text.init(m_renderer, fb, 14)) {
                std::fprintf(stderr, "[dbg] TTF fallback loaded: %s\n", fb);
                fontOK = true;
                break;
            }
        }
        if (!fontOK) {
            std::fprintf(stderr, "[dbg] TTF load failed (tried %s and fallbacks)\n", m_fontPath);
        }
    }



    // SDL/renderer sonrası, harita yüklemeden sonra uygun bir yere:
    Platform p;
    p.w = 96; p.h = 16;
    p.y = 32.f * 17.f;        // 17. satır hizası
    p.minX = 32.f * 18.f;     // soldaki sınır (tile 18)
    p.maxX = 32.f * 28.f;     // sağdaki sınır (tile 28)
    p.x = p.minX;             // başlangıç
    p.vx = 80.f;              // px/s sağa
    m_platforms.push_back(p);
    
    // Player start near top-left
    m_player.x = 64.f; m_player.y = 64.f; m_player.vx=0.f; m_player.vy=0.f; m_player.onGround=false;

    // Visual sprite
    if (m_atlas.loadGrid(m_renderer, "assets/atlas8x1.png", 32, 32, 0, 0)) {
        int total = m_atlas.frameCount();
        m_anim.setTotalFrames(total);
        std::fprintf(stderr, "[atlas] frames=%d\n", total);

        // atlas8x1.png varsayımı: [0..3]=Idle, [4..7]=Run, [6]=Jump, [7]=Fall
        // Kendi atlasına göre start/count ayarla
        m_anim.addClip("idle", 0, 2, 6.0f, true);
        m_anim.addClip("run", 2, 4, 8.0f, true);
        m_anim.addClip("jump", 6, 1, 12.0f, false);
        m_anim.addClip("fall", 7, 1, 8.0f, true);
        m_anim.addClip("land", 6, 1, 10.0f, false);

        m_anim.play("idle", true);

        m_animc.bind(&m_anim);
        m_animc.setClipNames("idle", "run", "jump", "fall", "land");
    }
    else {
        std::fprintf(stderr, "[warn] Atlas not found: assets/atlas8x1.png\n");
    }


    return true;
}

void Application::shutdown(){
    delete m_r2d; m_r2d=nullptr;
    Input::shutdown();
    if(m_renderer){ SDL_DestroyRenderer(m_renderer); m_renderer=nullptr; }
    if(m_window){ SDL_DestroyWindow(m_window); m_window=nullptr; }
    IMG_Quit(); SDL_Quit();
}

void Application::processEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
    }

    if (Input::keyPressed(SDL_SCANCODE_ESCAPE)) running = false;
    if (Input::keyPressed(SDL_SCANCODE_I))      m_paused = !m_paused;
    if (Input::keyPressed(SDL_SCANCODE_F))      m_follow = !m_follow;

    if (Input::keyPressed(SDL_SCANCODE_R)) {
        m_player.x = 64.f; m_player.y = 64.f; m_player.vx = 0.f; m_player.vy = 0.f; m_player.onGround = false;
        m_cam = {};
    }

    // Anim hız önayarları
    if (Input::keyPressed(SDL_SCANCODE_O)) { float f = m_anim.fps() * 0.5f; if (f < 1.0f) f = 1.0f; m_anim.setFPS(f); }
    if (Input::keyPressed(SDL_SCANCODE_P)) { float f = m_anim.fps() * 2.0f; if (f > 60.0f) f = 60.0f; m_anim.setFPS(f); }
    if (Input::keyPressed(SDL_SCANCODE_1)) m_anim.setFPS(4.0f);
    if (Input::keyPressed(SDL_SCANCODE_2)) m_anim.setFPS(8.0f);
    if (Input::keyPressed(SDL_SCANCODE_3)) m_anim.setFPS(12.0f);
    if (Input::keyPressed(SDL_SCANCODE_4)) m_anim.setFPS(24.0f);
    if (Input::keyPressed(SDL_SCANCODE_5)) m_anim.setFPS(48.0f);

    // Zoom
    if (Input::keyPressed(SDL_SCANCODE_Z)) { m_cam.zoom *= 0.8f;  if (m_cam.zoom < 0.5f) m_cam.zoom = 0.5f; }
    if (Input::keyPressed(SDL_SCANCODE_X)) { m_cam.zoom *= 1.25f; if (m_cam.zoom > 3.0f) m_cam.zoom = 3.0f; }

    // Hot reload
    if (Input::keyPressed(SDL_SCANCODE_F5)) {
        std::fprintf(stderr, "[hotreload] manual check\n");
        notifyHUD("Reload CHECK…", SDL_Color{ 70,130,200,255 }, 0.6f);
        m_res.check(true);
    }

    // Overlay toggle
    if (Input::keyPressed(SDL_SCANCODE_F1)) {
        m_dbgOverlay = !m_dbgOverlay;
        std::fprintf(stderr, "[dbg] overlay = %s\n", m_dbgOverlay ? "ON" : "OFF");
        notifyHUD(m_dbgOverlay ? "Debug ON" : "Debug OFF",
            SDL_Color{ 70,130,200,255 }, 0.8f);
    }

    // Space “bu framede basıldı” + yerde + S ile aşağı basmıyorken
    if (Input::keyPressed(SDL_SCANCODE_SPACE) && m_player.onGround && !Input::keyDown(SDL_SCANCODE_S)) {
        m_jumpTrigger = true;
    }

    // Layer toggles (BUNLAR FONKSİYON İÇİNDE KALMALI)
    if (Input::keyPressed(SDL_SCANCODE_7)) {
        m_dbgShowBG = !m_dbgShowBG;
        notifyHUD(m_dbgShowBG ? "BG ON" : "BG OFF", SDL_Color{ 180,180,180,255 }, 0.8f);
    }
    if (Input::keyPressed(SDL_SCANCODE_8)) {
        m_dbgShowFG = !m_dbgShowFG;
        notifyHUD(m_dbgShowFG ? "FG ON" : "FG OFF", SDL_Color{ 180,180,180,255 }, 0.8f);
    }
    if (Input::keyPressed(SDL_SCANCODE_9)) {
        m_dbgShowCol = !m_dbgShowCol;
        notifyHUD(m_dbgShowCol ? "COL ON" : "COL OFF", SDL_Color{ 200,120,60,255 }, 0.8f);
    }
}


void Application::update(double dt) {
    if (!m_paused) {
        bool left = Input::keyDown(SDL_SCANCODE_A) || Input::keyDown(SDL_SCANCODE_LEFT);
        bool right = Input::keyDown(SDL_SCANCODE_D) || Input::keyDown(SDL_SCANCODE_RIGHT);

        bool jumpPressed =
            Input::keyPressed(SDL_SCANCODE_SPACE) ||
            Input::keyPressed(SDL_SCANCODE_W) ||
            Input::keyPressed(SDL_SCANCODE_LSHIFT) ||
            Input::keyPressed(SDL_SCANCODE_RSHIFT);

        bool jumpHeld =
            Input::keyDown(SDL_SCANCODE_SPACE) ||
            Input::keyDown(SDL_SCANCODE_W) ||
            Input::keyDown(SDL_SCANCODE_LSHIFT) ||
            Input::keyDown(SDL_SCANCODE_RSHIFT);

        // Aşağı + zıplama: S veya ↓ basılıyken zıplama tuşu basıldıysa drop-through iste
        bool downHeld = Input::keyDown(SDL_SCANCODE_S) || Input::keyDown(SDL_SCANCODE_DOWN);
        bool dropRequest = downHeld && jumpPressed;



        // Fizik çağrısı (YENİ imza!)
        integrate(m_player, m_map, m_pp, (float)dt, left, right, jumpPressed, jumpHeld, dropRequest);

        // ① Yüz yönünü güncelle (anim flip için)
        if (std::fabs(m_player.vx) > 1.f) {
            m_faceRight = (m_player.vx >= 0.f);
        }

        // ② Animator Controller parametreleri
        Erlik::AnimParams ap;
        ap.onGround = m_player.onGround;
        ap.vx = m_player.vx;
        ap.vy = m_player.vy;
        ap.jumpTrigger = m_jumpTrigger;   // processEvents() içinde Space tetiklersin

        // ③ Controller + Animator update
        m_animc.update(dt, ap);
        m_anim.update(dt);

        // ④ Jump tetik bir-frame’liktir
        m_jumpTrigger = false;


        //UPDATE içinde süreyi azalt
        if (m_hudTimer > 0.f) {
            m_hudTimer = std::max(0.f, m_hudTimer - (float)dt);
        }

        // 1) Platformları hareket ettir
        for (auto& pl : m_platforms) {
            pl.x += pl.vx * (float)dt;
            if (pl.x < pl.minX) { pl.x = pl.minX; pl.vx = -pl.vx; }
            if (pl.x + pl.w > pl.maxX) { pl.x = pl.maxX - pl.w; pl.vx = -pl.vx; }
        }

        // 2) Üstten temas varsa “carry” uygula
        for (auto& pl : m_platforms) {
            const float px0 = m_player.x - m_player.halfW;
            const float px1 = m_player.x + m_player.halfW;
            const float py1 = m_player.y + m_player.halfH;
            const float prevBottom = m_player.prevY + m_player.halfH;

            const bool overlapX = (px1 > pl.x) && (px0 < pl.x + pl.w);
            const bool comingDownFromAbove = (m_player.vy >= 0.f) && (prevBottom <= pl.y);

            if (overlapX && comingDownFromAbove && py1 >= pl.y && py1 <= pl.y + 20.f) {
                // üstüne bin
                m_player.y = pl.y - m_player.halfH;
                m_player.vy = 0.f;
                m_player.onGround = true;

                // carry gücü: input yoksa 1.0, varsa 0.3
                bool hasInput = left || right;
                float carryStrength = hasInput ? 0.3f : 1.0f;
                m_player.x += pl.vx * (float)dt * carryStrength;
            }
        }

        // --- RUN FOOT DUST (yer + yeterli hız) ---
        if (m_player.onGround) {
            float speed = std::fabs(m_player.vx);
            if (speed > m_runDustMinSpd) {
                // hız faktörü: 0..1 (movespeed'e oranla)
                float k = std::clamp(speed / m_pp.moveSpeed, 0.0f, 1.0f);

                // hızlandıkça daha sık puf: 0.18s..0.06s aralığı
                m_runDustTimer -= (float)dt;
                if (m_runDustTimer <= 0.0f) {
                    m_runDustTimer = 0.18f - 0.12f * k;

                    // ayak hizası
                    float fx = m_player.x + (m_faceRight ? +m_player.halfW * 0.55f
                        : -m_player.halfW * 0.55f);
                    float fy = m_player.y + m_player.halfH - 2.0f;

                    // yön (karakterin baktığı tarafa doğru) ve temel hız
                    float dir = m_faceRight ? 1.0f : -1.0f;
                    float baseV = 80.0f + 120.0f * k;

                    // ana puf
                    m_fx.emitDust(fx, fy, 2 + int(2 * k), dir, baseV);

                    // hafif karşı tarafa da minik puf (daha doğal görünür)
                    m_fx.emitDust(fx, fy, 1, -dir, baseV * 0.5f);
                }
            }
            else {
                // yavaşken zamanlayıcıyı sıfırla
                m_runDustTimer = 0.0f;
            }
        }
        else {
            // havada: zamanlayıcıyı sıfırla
            m_runDustTimer = 0.0f;
        }


        // landing detection → shake + DUST
        if (!m_wasGround && m_player.onGround) {
            // Yaklaşık iniş hızı
            float vyApprox = (float)((m_player.y - m_player.prevY) / dt);
            float speed = std::max(std::fabs(vyApprox), std::fabs(m_player.vy));
            float impact = std::min(1.0f, speed / 300.0f);

            // oyuncunun ayak hizası
            float fx = m_player.x;
            float fy = m_player.y + m_player.halfH - 2.f;

            int   count = 8 + int(10 * impact);
            float dir = m_faceRight ? 1.f : -1.f;
            float baseV = 100.f + 180.f * impact;

            m_fx.emitDust(fx, fy, count, dir, baseV);
        }
        m_fx.update((float)dt);

        m_wasGround = m_player.onGround;



        m_r2d->beginFrame();   // her frame başı
        m_res.check(false);    // hot-reload dosya izleme (zaten eklemiştik)

    }

    // FPS hesapla (dt>0 ise). Exponential smoothing: new = (1/dt)*a + old*(1-a)
    if (dt > 0.0) {
        float inst = (float)(1.0 / dt);
        float a = m_fpsSmooth;          // 0.10 gibi
        if (m_currentFPS <= 0.0f) m_currentFPS = inst;  // ilk frame
        else m_currentFPS = m_currentFPS * (1.0f - a) + inst * a;
    }


    // Kamera
    // --- Camera follow (camera is TOP-LEFT in Renderer2D) ---
    int vw, vh; m_r2d->outputSize(vw, vh);
    float viewW = vw / m_cam.zoom;
    float viewH = vh / m_cam.zoom;

    // mevcut kameranın MERKEZİ
    float cx = m_cam.x + viewW * 0.5f;
    float cy = m_cam.y + viewH * 0.5f;

    // hedef (oyuncu merkezi)
    float tx = m_player.x;
    float ty = m_player.y;

    // dead-zone (zoom’a göre ölçekli)
    float hw = m_deadW / m_cam.zoom;
    float hh = m_deadH / m_cam.zoom;

    // dead-zone dışına çıktıysa merkez hedefini kaydır
    float dx = tx - cx;
    float dy = ty - cy;
    if (dx > hw) cx += (dx - hw);
    if (dx < -hw) cx += (dx + hw);
    if (dy > hh) cy += (dy - hh);
    if (dy < -hh) cy += (dy + hh);

    // yumuşak takip (dt’ye duyarlı)
    float lerp = 1.f - std::pow(1.f - m_camLerp, (float)(dt * 60.0));
    float newCx = m_cam.x + viewW * 0.5f + (cx - (m_cam.x + viewW * 0.5f)) * lerp;
    float newCy = m_cam.y + viewH * 0.5f + (cy - (m_cam.y + viewH * 0.5f)) * lerp;

    // MERKEZDEN tekrar TOP-LEFT'e çevir
    m_cam.x = newCx - viewW * 0.5f;
    m_cam.y = newCy - viewH * 0.5f;

    // (opsiyonel) dünyaya clamp
    if (m_worldW > 0 && m_worldH > 0) {
        float maxX = std::max(0.f, m_worldW - viewW);
        float maxY = std::max(0.f, m_worldH - viewH);
        m_cam.x = std::clamp(m_cam.x, 0.f, maxX);
        m_cam.y = std::clamp(m_cam.y, 0.f, maxY);
    }

    // --- camera shake offset (top-left semantiği)
    if (m_shake > 0.f) {
        // basit rastgele jitter
        float jx = ((float)std::rand() / RAND_MAX) * 2.f - 1.f;
        float jy = ((float)std::rand() / RAND_MAX) * 2.f - 1.f;
        m_cam.x += (jx * m_shakeAmp) / m_cam.zoom;
        m_cam.y += (jy * m_shakeAmp) / m_cam.zoom;

        // sönüm
        m_shake = std::max(0.f, m_shake - m_shakeDecay * (float)dt);
    }




    m_time += dt;
}



void Application::render(){
    m_r2d->setCamera(m_cam);
    m_r2d->clear(12, 12, 16, 255);

    if (m_dbgShowBG) m_tmj.drawBelowPlayer(*m_r2d);

    // Player
    if (const SDL_Rect* fr = m_atlas.frame(m_anim.index())) {
        SDL_RendererFlip flip = m_faceRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        m_r2d->drawTextureRegion(m_atlas.texture(), *fr, m_player.x, m_player.y, 1.6f, 0.0f, flip);
    }
    else {
        const int rectW = 24, rectH = 32;
        SDL_FRect r{ (m_player.x - m_cam.x) * m_cam.zoom - rectW * 0.5f,
                     (m_player.y - m_cam.y) * m_cam.zoom - rectH * 0.5f,
                     (float)rectW, (float)rectH };
        SDL_SetRenderDrawColor(m_renderer, 200, 200, 220, 255);
        SDL_RenderFillRectF(m_renderer, &r);
    }
    m_fx.draw(*m_r2d);

    // 3) FG katmanlar
    if (m_dbgShowFG) m_tmj.drawAbovePlayer(*m_r2d);

    // Collision heatmap (debug)
   // if (m_dbgCol && (m_dbgOverlay || m_dbgShowCol)) { /* varsa eski isim, aşağıdakiyle uyumlu yap */ }
    if (m_dbgOverlay) {
        int vw, vh; m_r2d->outputSize(vw, vh);

        // --------- Panel boyutu / konum ----------
        const int pad = 12;         // iç boşluk
        const int lh = 18;         // satır yüksekliği
        const int colW = 160;        // her sütunun genişliği
        const int cols = 2;
        const int rows = 5;          // kaç satır yazacağız
        const int panelW = pad * (cols + 1) + colW * cols;   // 12*3 + 160*2 = 336
        const int panelH = pad * 2 + lh * rows;              // 12*2 + 18*5 = 126

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 160);
        SDL_FRect bg{ (float)(vw - panelW - 10), 10.f, (float)panelW, (float)panelH };
        SDL_RenderFillRectF(m_renderer, &bg);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

        if (m_text.ready()) {
            SDL_Color w{ 220,220,220,255 };
            SDL_Color g{ 180,220,180,255 };
            SDL_Color y{ 200,200,120,255 };

            auto Lx = (int)bg.x + pad;           // sol sütun x
            auto Rx = Lx + colW + pad;           // sağ sütun x
            int  y0 = (int)bg.y + pad;
            char line[128];

            // satır 0
            std::snprintf(line, sizeof(line), "FPS: %.1f", m_currentFPS);
            m_text.draw(line, Lx, y0, w);
            std::snprintf(line, sizeof(line), "Anim: %s", m_animc.stateName());
            m_text.draw(line, Rx, y0, g);

            // satır 1
            y0 += lh;
            std::snprintf(line, sizeof(line), "DrawCalls: %d", m_r2d->drawCalls());
            m_text.draw(line, Lx, y0, w);

            // satır 2
            y0 += lh;
            std::snprintf(line, sizeof(line), "Coyote: %.0f ms", m_player.coyoteTimer * 1000.f);
            m_text.draw(line, Lx, y0, y);
            std::snprintf(line, sizeof(line), "BG[7]: %s", m_dbgShowBG ? "ON" : "OFF");
            m_text.draw(line, Rx, y0, w);

            // satır 3
            y0 += lh;
            std::snprintf(line, sizeof(line), "Buffer: %.0f ms", m_player.jumpBufferTimer * 1000.f);
            m_text.draw(line, Lx, y0, y);
            std::snprintf(line, sizeof(line), "FG[8]: %s", m_dbgShowFG ? "ON" : "OFF");
            m_text.draw(line, Rx, y0, w);

            // satır 4
            y0 += lh;
            std::snprintf(line, sizeof(line), "COL[9]: %s", m_dbgShowCol ? "ON" : "OFF");
            m_text.draw(line, Rx, y0, w);
        }
        else {
            // font yoksa yine de başlığa kısa özet yaz
            char title[128];
            std::snprintf(title, sizeof(title), "FPS %.1f | DC %d | Overlay (no font)",
                m_currentFPS, m_r2d->drawCalls());
            SDL_SetWindowTitle(m_window, title);
        }
    }





    //render() içinde platformu çiz (debug):
    for (auto& pl : m_platforms) {
        SDL_FRect r{
            (pl.x - m_cam.x) * m_cam.zoom,
            (pl.y - m_cam.y) * m_cam.zoom,
            pl.w * m_cam.zoom, pl.h * m_cam.zoom
        };
        SDL_SetRenderDrawColor(m_renderer, 180, 140, 80, 255);
        SDL_RenderFillRectF(m_renderer, &r);
    }

    // HUD title
    static double accum=0.0; static int frames=0; static double fps=0.0; int tile = m_map.tileSize();
    int underRow = (int)std::floor((m_player.y + m_player.halfH + 0.1f) / tile);
    int underL = m_map.get((int)std::floor((m_player.x - m_player.halfW + 1.0f) / tile), underRow);
    int underR = m_map.get((int)std::floor((m_player.x + m_player.halfW - 1.0f) / tile), underRow);
    accum += 1.0/60.0; frames++;
    if(frames>=30){
        // HUD title (DOĞRU SÜRÜM)
        char title[256];
        std::snprintf(title, sizeof(title),
            "Erlik | pos(%.1f,%.1f) v=(%.1f,%.1f) %s%s anim=%.1ff f=%d | under=[%d,%d] drop=%.2f",
            m_player.x, m_player.y, m_player.vx, m_player.vy,
            m_paused ? " | PAUSED" : "",
            m_player.onGround ? " | GROUND" : "",
            m_anim.fps(), m_anim.index(),
            underL, underR, m_player.dropTimer);
            if (m_hudTimer > 0.f && m_hudText[0] != '\0') {
                size_t len = std::strlen(title);
                std::snprintf(title + len, sizeof(title) - len, " | %s", m_hudText);
            }
        SDL_SetWindowTitle(m_window, title);

    }
    // clear() sonrasında, world çizmeden hemen önce (veya en sonda da olur):
    if (m_hudTimer > 0.f) {
        int vw, vh; m_r2d->outputSize(vw, vh);

        // Yumuşak görünüm için alfa: ilk 0.15s fade-in, son 0.3s fade-out
        float a = 1.0f;
        if (m_hudTimer < 0.3f) a = m_hudTimer / 0.3f;
        else if (m_hudTimer > 1.35f) a = std::clamp(1.5f - m_hudTimer, 0.f, 1.f);

        Uint8 alpha = (Uint8)std::round(220.f * a);

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

        // Arka plan siyah şerit
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, (Uint8)(140 * a));
        SDL_FRect bg{ 10.f, 10.f, (float)std::min(vw - 20, 420), 28.f };
        SDL_RenderFillRectF(m_renderer, &bg);

        // Sol kenarda renkli durum çubuğu (OK yeşil / FAIL kırmızı)
        SDL_SetRenderDrawColor(m_renderer, m_hudColor.r, m_hudColor.g, m_hudColor.b, alpha);
        SDL_FRect bar{ bg.x + 4.f, bg.y + 4.f, 8.f, bg.h - 8.f };
        SDL_RenderFillRectF(m_renderer, &bar);

        // Blend modunu geri al (isteğe bağlı)
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    m_r2d->present();
}

int Application::run(){
    if(!init()){ shutdown(); return -1; }
    bool running=true;
    Uint64 f=SDL_GetPerformanceFrequency(), last=SDL_GetPerformanceCounter();
    while(running){
        Uint64 now=SDL_GetPerformanceCounter(); Uint64 diff=now-last; last=now;
        double dt=(double)diff/(double)f; if(dt>0.1) dt=0.1;
        Input::beginFrame();
        SDL_PumpEvents();
        processEvents(running);
        Input::endFrame();
        update(dt);
        render();
    }
    shutdown();
    return 0;
}

void Application::notifyHUD(const char* msg, SDL_Color col, float seconds)
{
    const char* safe = msg ? msg : "";
#ifdef _MSC_VER
    // MSVC'de güvenli sürüm
    strncpy_s(m_hudText, sizeof(m_hudText), safe, _TRUNCATE);
#else
    // Diğer derleyiciler için güvenli kullanım
    std::strncpy(m_hudText, safe, sizeof(m_hudText) - 1);
    m_hudText[sizeof(m_hudText) - 1] = '\0';
#endif

    m_hudColor = col;
    m_hudTimer = seconds > 0.f ? seconds : 0.f;
}



} // namespace Erlik