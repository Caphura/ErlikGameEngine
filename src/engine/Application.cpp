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
    
    // Gamepad’i hazırla
    Input::initGamepads();

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

    m_spawnX = m_player.x;
    m_spawnY = m_player.y;


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
   
    Input::shutdownGamepads();
    if(m_renderer){ SDL_DestroyRenderer(m_renderer); m_renderer=nullptr; }
    if(m_window){ SDL_DestroyWindow(m_window); m_window=nullptr; }
    IMG_Quit(); SDL_Quit();
}

void Application::processEvents(bool& running) {
    SDL_Event e;
    Input::beginFrame();
    while (SDL_PollEvent(&e)) {
        Input::handleEvent(e);
        if (e.type == SDL_QUIT) running = false;
    }

    if (Input::keyPressed(SDL_SCANCODE_ESCAPE)) running = false;
    if (Input::keyPressed(SDL_SCANCODE_I))      m_paused = !m_paused;
    if (Input::keyPressed(SDL_SCANCODE_F))      m_follow = !m_follow;
    if (Input::padButtonPressed(SDL_CONTROLLER_BUTTON_START)) m_paused = !m_paused;


    if (Input::keyPressed(SDL_SCANCODE_R)) {
        m_player.x = m_spawnX;
        m_player.y = m_spawnY;
        m_player.vx = m_player.vy = 0.f;
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

        // ---- GAMEPAD KATKISI ----
        float padX = Input::padAxisLX();
        float padY = Input::padAxisLY();

        // hareket: stick veya dpad
        left = left || (padX < -0.25f) || Input::padButtonDown(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        right = right || (padX > 0.25f) || Input::padButtonDown(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

        // zıplama: A (veya Cross) tuşu
        jumpPressed = jumpPressed || Input::padButtonPressed(SDL_CONTROLLER_BUTTON_A);
        jumpHeld = jumpHeld || Input::padButtonDown(SDL_CONTROLLER_BUTTON_A);

        // aşağı + zıplama ile drop-through: stick aşağı ya da dpad down + A
        bool padDown = (padY > 0.45f) || Input::padButtonDown(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        downHeld = downHeld || padDown;
        dropRequest = dropRequest || (padDown && Input::padButtonPressed(SDL_CONTROLLER_BUTTON_A));

        // Overlay toggle
        // F1 / Pad Y — Debug Overlay
        static bool prevDbg = m_dbgOverlay;
        if (Input::keyPressed(SDL_SCANCODE_F1)) {
            m_dbgOverlay = !m_dbgOverlay;
        }
        if (prevDbg != m_dbgOverlay) {
            std::fprintf(stderr, "[dbg] overlay = %s\n", m_dbgOverlay ? "ON" : "OFF");
            notifyHUD(m_dbgOverlay ? "DBG ON" : "DBG OFF", SDL_Color{ 180,180,180,255 }, 0.8f);
            prevDbg = m_dbgOverlay;
        }

        // Fizik çağrısı (YENİ imza!)
        integrate(m_player, m_map, m_pp, (float)dt, left, right, jumpPressed, jumpHeld, dropRequest);

        auto overlap = [](float ax, float ay, float aw, float ah,
            float bx, float by, float bw, float bh)->bool {
                return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
        };

        // Player AABB (şimdi & önceki frame)
        float ax = m_player.x - m_player.halfW;
        float ay = m_player.y - m_player.halfH;
        float aw = m_player.halfW * 2.f;
        float ah = m_player.halfH * 2.f;
        float pax = m_player.prevX - m_player.halfW;
        float pay = m_player.prevY - m_player.halfH;

        for (const auto& tr : m_tmj.triggers()) {
            if (tr.once && m_triggersFired.count(tr.id)) continue;

            bool now = overlap(ax, ay, aw, ah, tr.x, tr.y, tr.w, tr.h);
            bool prev = overlap(pax, pay, aw, ah, tr.x, tr.y, tr.w, tr.h);

            if (now && !prev) {
                // === ENTER ===
                if (tr.type == "checkpoint") {
                    // Spawn'ı TRIGGER MERKEZİNE al (deterministik)
                    m_spawnX = tr.x + tr.w * 0.5f;
                    m_spawnY = tr.y + tr.h * 0.5f;
                    SDL_Log("TRIGGER checkpoint: id=%d name=%s", tr.id, tr.name.c_str());
                    SDL_SetWindowTitle(m_window, "Checkpoint!");
                    pushToast("Checkpoint!", 1.6f);
                }
                else if (tr.type == "door") {
                    // Hedef adına göre aynı harita içinde ışınlama
                    const auto* dst = m_tmj.findTriggerByName(tr.target);
                    if (dst) {
                        float nx = dst->x + dst->w * 0.5f;
                        float ny = dst->y + dst->h * 0.5f;
                        m_player.x = nx;
                        m_player.y = ny;
                        m_player.vx = 0.f;
                        m_player.vy = 0.f;
                        SDL_Log("TRIGGER door: %s -> %s (teleport)", tr.name.c_str(), tr.target.c_str());
                        SDL_SetWindowTitle(m_window, (std::string("Door -> ") + tr.target).c_str());
                        pushToast("Door: " + tr.name + " -> " + tr.target, 1.8f);


                        // Küçük bir kamera sarsıntısı (keyif için)
                        m_shake = std::min(1.0f, m_shake + 0.35f);
                    }
                    else {
                        SDL_Log("WARN: door target not found: %s", tr.target.c_str());
                    }
                }
                else { // "region" ve diğer custom türler
                    if (!tr.message.empty()) {
                        pushToast(tr.message, 2.2f);
                    }
                    // zoom property varsa uygula (0 ise yok say)
                    if (tr.zoom > 0.0f) {
                        m_cam.zoom = std::clamp(tr.zoom, 0.25f, 3.0f);
                    }
                    SDL_Log("TRIGGER %s: name=%s zoom=%.2f", tr.type.c_str(), tr.name.c_str(), tr.zoom);
                }


                if (tr.once) m_triggersFired.insert(tr.id);
            }
        }

        // --- HUD toasts: zaman ilerlet & süresi dolanları at
        for (auto& it : m_toasts) it.t += (float)dt;
        while (!m_toasts.empty() && m_toasts.front().t > m_toasts.front().dur) {
            m_toasts.pop_front();
        }


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
                float k = std::clamp(speed / m_pp.moveSpeed, 0.0f, 1.0f);

                // üretim aralığını SEYRELT: 0.24s..0.12s
                m_runDustTimer -= (float)dt;
                if (m_runDustTimer <= 0.0f) {
                    m_runDustTimer = 0.24f - 0.12f * k;

                    // ARKADA spawn: karakterin baktığı yönün tersinde offset
                    float behind = (m_faceRight ? -1.f : +1.f) * (m_player.halfW * 0.65f);
                    float fx = m_player.x + behind;
                    float fy = m_player.y + m_player.halfH - 2.0f;

                    // yön: ARKAYA doğru (baktığının tersi)
                    float dirBack = (m_faceRight ? -1.f : +1.f);

                    // sayıyı da düşük tut
                    int count = 1 + (k > 0.65f ? 1 : 0);

                    // yeni koşu tozu yayıcıyı kullan
                    m_fx.emitFootDust(fx, fy, count, dirBack);
                }
            }
            else {
                m_runDustTimer = 0.0f;
            }
        }
        else {
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

            int   total = 8 + int(10 * impact);
            int   each = std::max(1, total / 2);
            float baseV = 90.f + 140.f * impact;   // biraz daha yumuşak hız

            // iki yana bulut
            m_fx.emitDust(fx, fy, each, +1.f, baseV);
            m_fx.emitDust(fx, fy, each, -1.f, baseV);

            // çok küçük bir merkez pufu: “toz yükseliyor” hissi
            m_fx.emitFootDust(fx, fy, 1, +1.f);
            m_fx.emitFootDust(fx, fy, 1, -1.f);
            Input::rumble(18000, 28000, (Uint32)(80 + 120 * impact)); // 80–200ms hafif rumble
        }

        m_fx.update((float)dt);

        m_wasGround = m_player.onGround;



        
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
    m_r2d->beginFrame();
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

        if (m_dbgShowCol) {
            m_tmj.drawTriggersDebug(*m_r2d);
        }


        if (m_text.ready())  // === DEBUG OVERLAY TEXT ===
        {
            // Renkler
            const SDL_Color cWhite{ 255,255,255,255 };
            const SDL_Color cGreen{ 80,220,120,255 };
            const SDL_Color cYellow{ 255,220,  0,255 };

            // Panel içi yerleşim: bg + padding kullan
            const int xL = (int)bg.x + pad;           // sol sütun X
            const int xR = xL + colW + pad;           // sağ sütun X
            int yL = (int)bg.y + pad;                 // sol sütun ilk satır Y
            int yR = (int)bg.y + pad;                 // sağ sütun ilk satır Y
            const int dy = lh;                        // satır aralığı

            char line[256];

            // ---- SOL SÜTUN ----
            std::snprintf(line, sizeof(line), "FPS: %.1f", m_currentFPS);
            m_text.draw(line, xL, yL, cWhite, 1.0f);  yL += dy;

            std::snprintf(line, sizeof(line), "DrawCalls: %d", m_r2d->drawCalls());
            m_text.draw(line, xL, yL, cGreen, 1.0f);  yL += dy;

            std::snprintf(line, sizeof(line), "Player");
            m_text.draw(line, xL, yL, cYellow, 1.0f); yL += dy;

            std::snprintf(line, sizeof(line), "x=%.1f", m_player.x);
            m_text.draw(line, xL, yL, cYellow, 1.0f); yL += dy;

            std::snprintf(line, sizeof(line), "y=%.1f", m_player.y);
            m_text.draw(line, xL, yL, cYellow, 1.0f); yL += dy;

            // ---- SAĞ SÜTUN ----
            std::snprintf(line, sizeof(line), "Zoom: %.2f", m_cam.zoom);
            m_text.draw(line, xR, yR, cWhite, 1.0f);  yR += dy;

            std::snprintf(line, sizeof(line), "Layers: BG[%s] FG[%s]",
                m_dbgShowBG ? "on" : "off",
                m_dbgShowFG ? "on" : "off");
            m_text.draw(line, xR, yR, cGreen, 1.0f);  yR += dy;

            std::snprintf(line, sizeof(line), "COL/Triggers: %s", m_dbgShowCol ? "on" : "off");
            m_text.draw(line, xR, yR, cYellow, 1.0f); yR += dy;
        }
        else {
            // font yoksa kısa özet başlığa
            char title[128];
            std::snprintf(title, sizeof(title), "FPS %.1f | DC %d | Overlay (no font)",
                m_currentFPS, m_r2d->drawCalls());
            SDL_SetWindowTitle(m_window, title);
        }

    }

    // --- HUD toasts (sol üst, gölge + alfa ile fade)
    int x = 12;
    int y = 12;
    for (size_t i = 0; i < m_toasts.size(); ++i) {
        const auto& tt = m_toasts[i];

        // Basit fade-out: son 0.35 sn'de sönsün
        float a = 1.0f;
        const float fade = 0.35f;
        if (tt.t > tt.dur - fade) {
            a = std::max(0.f, 1.f - (tt.t - (tt.dur - fade)) / fade);
        }

        // Alfa’ları hesapla
        Uint8 aText = (Uint8)std::round(255.f * a);
        Uint8 aShadow = (Uint8)std::round(200.f * a);

        // Gölge (1px offset)
        m_text.draw(tt.text.c_str(), x + 1, y + 1 + (int)i * 18, SDL_Color{ 0,0,0,aShadow }, 1.0f);
        m_text.draw(tt.text.c_str(), x, y + (int)i * 18, SDL_Color{ 255,255,255,aText }, 1.0f);

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
        Input::beginFrame();
        Uint64 now=SDL_GetPerformanceCounter(); Uint64 diff=now-last; last=now;
        double dt=(double)diff/(double)f; if(dt>0.1) dt=0.1;
        SDL_PumpEvents();
        processEvents(running);
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

void Application::pushToast(const std::string& msg, float dur) {
    if (msg.empty()) return;
    Toast t; t.text = msg; t.t = 0.f; t.dur = dur;
    m_toasts.push_back(std::move(t));
}



} // namespace Erlik