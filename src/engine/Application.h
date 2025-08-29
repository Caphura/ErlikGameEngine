#pragma once
#include <cstdint>
#include <vector>
#include "Renderer2D.h"
#include "Texture.h"
#include "SpriteAtlas.h"
#include "Animator.h"
#include "Tilemap.h"
#include "Physics.h"
#include "TMJMap.h"
#include "ResourceManager.h"
#include <SDL.h>   // SDL_Color için
#include "TextRenderer.h"
#include "AnimatorController.h"
#include "Particles.h"
#include <unordered_set>
#include <deque>


struct SDL_Window;
struct SDL_Renderer;

namespace Erlik {

    struct Platform {
        float x = 0.f, y = 0.f, w = 96.f, h = 16.f;
        float vx = 0.f, vy = 0.f;
        float minX = 0.f, maxX = 0.f; // yatay salýným sýnýrlarý
    };

    class Application {
    public:
        int run();

    private:
        bool init();
        void shutdown();
        void processEvents(bool& running);
        void update(double dt);
        void render();
        // Debug overlay
        bool m_dbgOverlay = false;
        bool m_dbgShowBG = true;
        bool m_dbgShowFG = true;
        bool m_dbgShowCol = false; // collision heatmap
        bool m_dbgOverlayPrev = false;   // sadece deðiþtiðinde logla

        struct Toast {
            std::string text;
            float t = 0.f;          // geçen süre
            float dur = 2.5f;       // ekranda kalma süresi (sn)
        };
        std::deque<Toast> m_toasts; // HUD toast kuyruðu
        void pushToast(const std::string& msg, float dur = 2.5f);
        

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        int m_width = 1280;
        int m_height = 720;

        TextRenderer m_text;
        const char* m_fontPath = "assets/DejaVuSans.ttf"; // deðiþtirilebilir

        float m_currentFPS = 0.0f;   // FPS göstergesi (exponential smoothing)
        float m_fpsSmooth = 0.10f;   // 0.1 iyi bir baþlangýç

        // Kaynak & hot reload
        ResourceManager m_res;
        std::string     m_tmjPath; // izlediðimiz tmj dosyasý
        
        // --- HUD: HotReload bildirimi ---
        float     m_hudTimer = 0.f;         // saniye
        SDL_Color m_hudColor{ 0,0,0,0 };     // banner rengi
        char      m_hudText[160]{ 0 };       // baþlýða eklenecek kýsa metin
        void notifyHUD(const char* msg, SDL_Color col, float seconds = 1.5f);
        
        //TMJMap
        TMJMap      m_tmj;        // çok katman çizim
        Tilemap     m_map;        // fizik için grid (collision + oneway)

        // Render
        Renderer2D* m_r2d = nullptr;
        Camera2D    m_cam;

        // World
        Player        m_player;
        PhysicsParams m_pp;

        // World size (pixel) — TMJ/CSV yüklenince set edilir
        float m_worldW = 0.f;
        float m_worldH = 0.f;

        //Particles
        Erlik::ParticleSystem m_fx;

        // Visuals
        SpriteAtlas m_atlas;
        Animator    m_anim;

        // Anim state (YALNIZCA ÜYE OLARAK!)
        Erlik::AnimatorController m_animc;
        bool  m_faceRight = true;   // sprite saða bakýyor mu?
        bool  m_jumpTrigger = false;   // bu framede Space tetiklendiyse
        bool  m_wasGround = false;   // (opsiyonel) landing tespiti için

        // State
        bool  m_paused = false;
        bool  m_follow = true;
        double m_time = 0.0;

        // Platforms
        std::vector<Platform> m_platforms;

        //Cam Lerp
        float m_camLerp = 0.15f;     // 0..1  (takip hýz)
        float m_deadW = 80.f;      // dead-zone yarý-geniþlik (px, world)
        float m_deadH = 60.f;      // dead-zone yarý-yükseklik (px, world)

        // Camera shake
        float m_shake = 0.0f;  // 0..1
        float m_shakeDecay = 2.5f;  // s^-1
        float m_shakeAmp = 2.0f;  // px @ zoom=1

        // Landing feedback (prev grounded state)
        bool  m_prevOnGround = false;

        // --- Door FX (fade + teleport) ---
        bool  m_doorFxActive = false;
        int   m_doorFxPhase = 0;      // 0=idle, 1=fade-in, 2=fade-out
        float m_doorFxT = 0.f;    // current phase timer (sec)
        float m_doorFadeIn = 0.12f;  // sec
        float m_doorFadeOut = 0.14f;  // sec
        float m_doorAlpha = 0.f;    // 0..1 (render overlay)
        bool  m_doorTeleportPending = false;
        float m_doorTeleportY = 0.f;
        float m_doorTeleportX = 0.f;

        // Run foot dust (timer-based)
        float m_runDustTimer = 0.0f;   // geri sayým
        float m_runDustMinSpd = 35.0f; // tetik eþiði (px/s)

        // Trigger state
        std::unordered_set<int> m_triggersFired; // once=true olanlar için
        float m_spawnX = 0.f, m_spawnY = 0.f;    // checkpoint noktasý
        int   m_saveSlot = 1;                    // aktif kayýt slotu (1..3)

        // Inventory: anahtarlar
        std::unordered_set<std::string> m_keys;

        // Music region state
        int         m_activeMusicRegionId = -1;
        std::string m_musicCurrent;      // þu an çalan (biz takip ediyoruz)
        std::string m_musicBeforeRegion; // region’a girmeden önce çalan


    };

} // namespace Erlik
