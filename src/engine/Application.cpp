#include "Application.h"
#include "Input.h"
#include <SDL.h>
#include <cstdio>

namespace Erlik {

    bool Application::init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
            std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
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

        // Baþlangýç pozisyonu merkeze
        m_posX = m_width * 0.5f;
        m_posY = m_height * 0.5f;

        Input::init();
        return true;
    }

    void Application::shutdown() {
        Input::shutdown();
        if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
        if (m_window) { SDL_DestroyWindow(m_window);     m_window = nullptr; }
        SDL_Quit();
    }

    void Application::processEvents(bool& running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        if (Input::keyPressed(SDL_SCANCODE_ESCAPE)) running = false;
        if (Input::keyPressed(SDL_SCANCODE_SPACE))  m_paused = !m_paused;
        if (Input::keyPressed(SDL_SCANCODE_R)) {
            m_posX = m_width * 0.5f;
            m_posY = m_height * 0.5f;
        }
    }

    void Application::update(double dt) {
        if (m_paused) return;

        const float speed = 300.0f; // px/s
        if (Input::keyDown(SDL_SCANCODE_LEFT) || Input::keyDown(SDL_SCANCODE_A)) m_posX -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_RIGHT) || Input::keyDown(SDL_SCANCODE_D)) m_posX += speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_UP) || Input::keyDown(SDL_SCANCODE_W)) m_posY -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_DOWN) || Input::keyDown(SDL_SCANCODE_S)) m_posY += speed * (float)dt;

        // pencere içine kýsýtla
        const float half = 40.0f;
        if (m_posX < half) m_posX = half;
        if (m_posY < half) m_posY = half;
        if (m_posX > m_width - half) m_posX = m_width - half;
        if (m_posY > m_height - half) m_posY = m_height - half;

        m_time += dt;
    }

    void Application::render() {
        SDL_SetRenderDrawColor(m_renderer, 12, 12, 16, 255);
        SDL_RenderClear(m_renderer);

        const int rectW = 80;
        const int rectH = 80;

        SDL_Rect r{
            static_cast<int>(m_posX - rectW * 0.5f),
            static_cast<int>(m_posY - rectH * 0.5f),
            rectW, rectH
        };

        // Mouse-sol týk anýnda kýsa bir "flash" efekti
        if (Input::mousePressed(SDL_BUTTON_LEFT)) {
            SDL_SetRenderDrawColor(m_renderer, 255, 220, 120, 255);
        }
        else {
            SDL_SetRenderDrawColor(m_renderer, 200, 200, 220, 255);
        }
        SDL_RenderFillRect(m_renderer, &r);

        SDL_RenderPresent(m_renderer);
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
        int fpsFrames = 0;
        double currentFPS = 0.0;

        while (running) {
            Uint64 now = SDL_GetPerformanceCounter();
            Uint64 diff = now - last;
            last = now;

            double dt = static_cast<double>(diff) / static_cast<double>(perfFreq);
            if (dt > 0.1) dt = 0.1;

            Input::beginFrame();
            SDL_PumpEvents();          // güvence
            processEvents(running);
            Input::endFrame();

            update(dt);
            render();

            fpsTimer += dt;
            fpsFrames++;
            if (fpsTimer >= 0.5) {
                currentFPS = fpsFrames / fpsTimer;
                fpsFrames = 0;
                fpsTimer = 0.0;
                char title[128];
                std::snprintf(title, sizeof(title), "ErlikGameEngine  |  %.1f FPS%s",
                    currentFPS, m_paused ? " | PAUSED" : "");
                SDL_SetWindowTitle(m_window, title);
            }
        }

        shutdown();
        return 0;
    }

} // namespace Erlik
