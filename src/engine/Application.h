#pragma once
#include <cstdint>
#include "Renderer2D.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Renderer;

namespace Erlik {

    class Application {
    public:
        Application() = default;
        ~Application() = default;

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

        double m_time = 0.0;

        // Demo state
        float m_posX = 0.f, m_posY = 0.f, m_rot = 0.f, m_scale = 2.f;
        bool  m_paused = false;
        bool  m_follow = false; // F ile aç/kapat: kamera takip modu

        Renderer2D* m_r2d = nullptr;
        Texture     m_tex;
        Camera2D    m_cam;
    };

} // namespace Erlik
