#include "Application.h"
#include "Input.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>

namespace Erlik {

    bool Application::init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
            std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }

        // Sadece PNG zorunlu: JPEG'i þart koþmayalým
        int flags = IMG_INIT_PNG;
        int initted = IMG_Init(flags);
        if ((initted & IMG_INIT_PNG) == 0) {
            std::fprintf(stderr, "IMG_Init failed (PNG not available): %s\n", IMG_GetError());
            return false;
        }

        m_window = SDL_CreateWindow(
            "ErlikGameEngine",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            m_width, m_height,
            SDL_WINDOW_SHOWN
        );
        if (!m_window) {
            std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        if (!m_renderer) {
            std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
            return false;
        }

        // Baþlangýç state
        m_posX = m_width * 0.5f;
        m_posY = m_height * 0.5f;
        m_rot = 0.0f;
        m_scale = 2.0f;
        m_paused = false;
        m_cam = {};

        Input::init();

        // Varsa sprite'ý yükle; yoksa render'da yedek kare çizeriz
        if (!m_tex.loadFromFile(m_renderer, "assets/checker.png")) {
            std::fprintf(stderr, "[warn] Texture not found: assets/checker.png (fallback rect will be used)\n");
        }

        // Basit renderer sarmalayýcýsý
        m_r2d = new Renderer2D(m_renderer);

        return true;
    }

    void Application::shutdown() {
        delete m_r2d; m_r2d = nullptr;
        m_tex.destroy();
        Input::shutdown();
        if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
        if (m_window) { SDL_DestroyWindow(m_window);     m_window = nullptr; }
        IMG_Quit();
        SDL_Quit();
    }

    void Application::processEvents(bool& running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        // Kýsa yol tuþlarý
        if (Input::keyPressed(SDL_SCANCODE_ESCAPE)) running = false;
        if (Input::keyPressed(SDL_SCANCODE_SPACE))  m_paused = !m_paused;
        if (Input::keyPressed(SDL_SCANCODE_R)) {
            m_posX = m_width * 0.5f;
            m_posY = m_height * 0.5f;
            m_rot = 0.0f;
            m_scale = 2.0f;
            m_cam = {};
        }
        if (Input::keyPressed(SDL_SCANCODE_F)) {
            m_follow = !m_follow;
        }

        // Ölçek ve rotasyon
        if (Input::keyDown(SDL_SCANCODE_Q)) m_rot -= 120.0f / 60.0f;
        if (Input::keyDown(SDL_SCANCODE_E)) m_rot += 120.0f / 60.0f;
        if (Input::keyDown(SDL_SCANCODE_Z)) m_scale -= 1.0f / 60.0f;
        if (Input::keyDown(SDL_SCANCODE_X)) m_scale += 1.0f / 60.0f;
        if (m_scale < 0.2f) m_scale = 0.2f;
        if (m_scale > 8.0f) m_scale = 8.0f;

        // Kamera zoom
        if (Input::keyDown(SDL_SCANCODE_MINUS))   m_cam.zoom *= 0.99f;
        if (Input::keyDown(SDL_SCANCODE_EQUALS))  m_cam.zoom *= 1.01f;
        if (m_cam.zoom < 0.25f) m_cam.zoom = 0.25f;
        if (m_cam.zoom > 4.0f)  m_cam.zoom = 4.0f;
    }

    void Application::update(double dt) {
        if (m_paused) return;

        const float speed = 300.0f;
        if (Input::keyDown(SDL_SCANCODE_LEFT) || Input::keyDown(SDL_SCANCODE_A)) m_posX -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_RIGHT) || Input::keyDown(SDL_SCANCODE_D)) m_posX += speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_UP) || Input::keyDown(SDL_SCANCODE_W)) m_posY -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_DOWN) || Input::keyDown(SDL_SCANCODE_S)) m_posY += speed * (float)dt;

        // Kamera hedefi (takip)
        m_cam.x = m_posX - m_width * 0.5f / m_cam.zoom;
        m_cam.y = m_posY - m_height * 0.5f / m_cam.zoom;

        m_time += dt;
        // update(double dt) içinde, WASD ile m_posX/m_posY güncelledikten sonra:
        if (m_follow) {
            // Takip: objeyi merkezde tut
            m_cam.x = m_posX - m_width * 0.5f / m_cam.zoom;
            m_cam.y = m_posY - m_height * 0.5f / m_cam.zoom;
        }
        else {
            // Sabit kamera: hareketi ekranda gör
            m_cam.x = 0.0f;
            m_cam.y = 0.0f;
        }

    }

    void Application::render() {
        m_r2d->setCamera(m_cam);
        m_r2d->clear(12, 12, 16, 255);

        if (m_tex.sdl()) {
            // Sprite varsa onu çiz
            m_r2d->drawTexture(m_tex, m_posX, m_posY, m_scale, m_rot);
        }
        else {
            // Yedek kare (sprite yoksa da hareketi gör)
            const int rectW = 80, rectH = 80;
            SDL_FRect r{
                (m_posX - m_cam.x) * m_cam.zoom - rectW * 0.5f,
                (m_posY - m_cam.y) * m_cam.zoom - rectH * 0.5f,
                (float)rectW, (float)rectH
            };
            SDL_SetRenderDrawColor(m_renderer, 200, 200, 220, 255);
            SDL_RenderFillRectF(m_renderer, &r);
        }

        m_r2d->present();
    }

    int Application::run() {
        if (!init()) {
            shutdown();
            return -1;
        }

        bool running = true;

        Uint64 perfFreq = SDL_GetPerformanceFrequency();
        Uint64 last = SDL_GetPerformanceCounter();

        double fpsTimer = 0.0;
        int    fpsFrames = 0;
        double currentFPS = 0.0;

        while (running) {
            Uint64 now = SDL_GetPerformanceCounter();
            Uint64 diff = now - last;
            last = now;

            double dt = (double)diff / (double)perfFreq;
            if (dt > 0.1) dt = 0.1;

            // Input çerçevesi
            Input::beginFrame();
            SDL_PumpEvents();
            processEvents(running);
            Input::endFrame();

            update(dt);
            render();

            // Baþlýk/FPS/pozisyon
            fpsTimer += dt;
            fpsFrames++;
            if (fpsTimer >= 0.5) {
                currentFPS = fpsFrames / fpsTimer;
                fpsFrames = 0;
                fpsTimer = 0.0;

                // run() içindeki FPS baþlýðý set edildiði yerde:
                char title[256];
                std::snprintf(title, sizeof(title),
                    "ErlikGameEngine | %.1f FPS%s | x=%.1f y=%.1f%s",
                    currentFPS, m_paused ? " | PAUSED" : "",
                    m_posX, m_posY,
                    m_follow ? " | CAM=FOLLOW" : " | CAM=FIXED"
                );
                SDL_SetWindowTitle(m_window, title);
            }
        }

        shutdown();
        return 0;
    }

} // namespace Erlik
