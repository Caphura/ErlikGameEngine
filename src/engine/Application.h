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
#include <SDL.h>   // SDL_Color i�in
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
        float minX = 0.f, maxX = 0.f; // yatay sal�n�m s�n�rlar�
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
        bool m_dbgOverlayPrev = false;   // sadece de�i�ti�inde logla

        struct Toast {
            std::string text;
            float t = 0.f;          // ge�en s�re
            float dur = 2.5f;       // ekranda kalma s�resi (sn)
        };
        std::deque<Toast> m_toasts; // HUD toast kuyru�u
        void pushToast(const std::string& msg, float dur = 2.5f);
        

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        int m_width = 1280;
        int m_height = 720;

        TextRenderer m_text;
        const char* m_fontPath = "assets/DejaVuSans.ttf"; // de�i�tirilebilir

        float m_currentFPS = 0.0f;   // FPS g�stergesi (exponential smoothing)
        float m_fpsSmooth = 0.10f;   // 0.1 iyi bir ba�lang��

        // Kaynak & hot reload
        ResourceManager m_res;
        std::string     m_tmjPath; // izledi�imiz tmj dosyas�
        
        // --- HUD: HotReload bildirimi ---
        float     m_hudTimer = 0.f;         // saniye
        SDL_Color m_hudColor{ 0,0,0,0 };     // banner rengi
        char      m_hudText[160]{ 0 };       // ba�l��a eklenecek k�sa metin
        void notifyHUD(const char* msg, SDL_Color col, float seconds = 1.5f);
        
        //TMJMap
        TMJMap      m_tmj;        // �ok katman �izim
        Tilemap     m_map;        // fizik i�in grid (collision + oneway)

        // Render
        Renderer2D* m_r2d = nullptr;
        Camera2D    m_cam;

        // World
        Player        m_player;
        PhysicsParams m_pp;

        // World size (pixel) � TMJ/CSV y�klenince set edilir
        float m_worldW = 0.f;
        float m_worldH = 0.f;

        //Particles
        Erlik::ParticleSystem m_fx;

        // Visuals
        SpriteAtlas m_atlas;
        Animator    m_anim;

        // Anim state (YALNIZCA �YE OLARAK!)
        Erlik::AnimatorController m_animc;
        bool  m_faceRight = true;   // sprite sa�a bak�yor mu?
        bool  m_jumpTrigger = false;   // bu framede Space tetiklendiyse
        bool  m_wasGround = false;   // (opsiyonel) landing tespiti i�in

        // State
        bool  m_paused = false;
        bool  m_follow = true;
        double m_time = 0.0;

        // Platforms
        std::vector<Platform> m_platforms;

        //Cam Lerp
        float m_camLerp = 0.15f;     // 0..1  (takip h�z)
        float m_deadW = 80.f;      // dead-zone yar�-geni�lik (px, world)
        float m_deadH = 60.f;      // dead-zone yar�-y�kseklik (px, world)

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
        float m_runDustTimer = 0.0f;   // geri say�m
        float m_runDustMinSpd = 35.0f; // tetik e�i�i (px/s)

        // Trigger state
        std::unordered_set<int> m_triggersFired; // once=true olanlar i�in
        float m_spawnX = 0.f, m_spawnY = 0.f;    // checkpoint noktas�
        int   m_saveSlot = 1;                    // aktif kay�t slotu (1..3)

        // Inventory: anahtarlar
        std::unordered_set<std::string> m_keys;

        // Music region state
        int         m_activeMusicRegionId = -1;
        std::string m_musicCurrent;      // �u an �alan (biz takip ediyoruz)
        std::string m_musicBeforeRegion; // region�a girmeden �nce �alan


    };

} // namespace Erlik
