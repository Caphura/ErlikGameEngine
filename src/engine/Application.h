#pragma once
#include <cstdint>

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

        double m_time = 0.0; // seconds since start

        // Input demo state
        float m_posX = 0.0f;
        float m_posY = 0.0f;
        bool  m_paused = false;
    };

} // namespace Erlik
