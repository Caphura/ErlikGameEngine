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

    };

} // namespace Erlik
