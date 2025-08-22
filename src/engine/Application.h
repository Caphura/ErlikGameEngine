#pragma once
#include <cstdint>
#include <vector>
#include "Renderer2D.h"
#include "Texture.h"
#include "SpriteAtlas.h"
#include "Animator.h"
#include "Tilemap.h"
#include "Physics.h"

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

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        int m_width = 1280;
        int m_height = 720;

        // Render
        Renderer2D* m_r2d = nullptr;
        Camera2D    m_cam;

        // World
        Tilemap       m_map;
        Player        m_player;
        PhysicsParams m_pp;

        // Visuals
        SpriteAtlas m_atlas;
        Animator    m_anim;

        // State
        bool  m_paused = false;
        bool  m_follow = true;
        double m_time = 0.0;

        // Platforms
        std::vector<Platform> m_platforms;
    };

} // namespace Erlik
