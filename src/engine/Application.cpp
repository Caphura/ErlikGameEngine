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
    int flags = IMG_INIT_PNG;
    if ((IMG_Init(flags) & flags) != flags) {
        std::fprintf(stderr, "IMG_Init failed (PNG not available): %s\n", IMG_GetError());
        return false;
    }

    m_window = SDL_CreateWindow("ErlikGameEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_SHOWN);
    if (!m_window) { std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); return false; }
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer){ std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError()); return false; }

    m_r2d = new Renderer2D(m_renderer);

    m_posX = m_width * 0.5f;
    m_posY = m_height * 0.5f;

    Input::init();

    // Fallback sprite
    m_tex.loadFromFile(m_renderer, "assets/checker.png");

    // Atlas (karakter)
    if (m_atlas.loadGrid(m_renderer, "assets/atlas8x1.png", 32, 32, 0, 0)) {
        m_anim.set(m_atlas.frameCount(), 8.0f, true);
    }

    // Tilemap yükle
    if (!m_map.loadCSV("assets/level1.csv")) {
        std::fprintf(stderr, "[warn] level1.csv not found\n");
    }
    if (!m_map.loadTileset(m_renderer, "assets/tileset.png", 32, 32, 0, 0)) {
        std::fprintf(stderr, "[warn] tileset.png not found\n");
    }

    return true;
}

void Application::shutdown() {
    delete m_r2d; m_r2d = nullptr;
    m_tex.destroy();
    Input::shutdown();
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window = nullptr; }
    IMG_Quit();
    SDL_Quit();
}

void Application::processEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
    }
    if (Input::keyPressed(SDL_SCANCODE_ESCAPE)) running = false;
    if (Input::keyPressed(SDL_SCANCODE_SPACE))  m_paused = !m_paused;
    if (Input::keyPressed(SDL_SCANCODE_F))      m_follow = !m_follow;
    if (Input::keyPressed(SDL_SCANCODE_R)) {
        m_posX = m_width*0.5f; m_posY = m_height*0.5f; m_rot=0.f; m_scale=2.f; m_cam={};
    }
    // Anim hız kısa yolları
    if (Input::keyPressed(SDL_SCANCODE_O) || Input::keyPressed(SDL_SCANCODE_LEFTBRACKET)) {
        float f = m_anim.fps() * 0.5f; if (f < 1.0f) f = 1.0f; m_anim.setFPS(f);
    }
    if (Input::keyPressed(SDL_SCANCODE_P) || Input::keyPressed(SDL_SCANCODE_RIGHTBRACKET)) {
        float f = m_anim.fps() * 2.0f; if (f > 60.0f) f = 60.0f; m_anim.setFPS(f);
    }
    if (Input::keyPressed(SDL_SCANCODE_1)) m_anim.setFPS(4.0f);
    if (Input::keyPressed(SDL_SCANCODE_2)) m_anim.setFPS(8.0f);
    if (Input::keyPressed(SDL_SCANCODE_3)) m_anim.setFPS(12.0f);
    if (Input::keyPressed(SDL_SCANCODE_4)) m_anim.setFPS(24.0f);
    if (Input::keyPressed(SDL_SCANCODE_5)) m_anim.setFPS(48.0f);
}

void Application::update(double dt) {
    if (!m_paused) {
        const float speed = 300.f;
        if (Input::keyDown(SDL_SCANCODE_LEFT)  || Input::keyDown(SDL_SCANCODE_A)) m_posX -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_RIGHT) || Input::keyDown(SDL_SCANCODE_D)) m_posX += speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_UP)    || Input::keyDown(SDL_SCANCODE_W)) m_posY -= speed * (float)dt;
        if (Input::keyDown(SDL_SCANCODE_DOWN)  || Input::keyDown(SDL_SCANCODE_S)) m_posY += speed * (float)dt;
        m_anim.update(dt);
    }

    if (m_follow) {
        m_cam.x = m_posX - m_width  * 0.5f / m_cam.zoom;
        m_cam.y = m_posY - m_height * 0.5f / m_cam.zoom;
    } else {
        m_cam.x = 0.f; m_cam.y = 0.f;
    }

    m_time += dt;
}

void Application::render() {
    m_r2d->setCamera(m_cam);
    m_r2d->clear(12,12,16,255);

    // Tilemap'i önce çiz (zemin)
    m_map.draw(*m_r2d);

    // Grid üstüne hafif
    m_r2d->drawGrid(64, 50,50,60,120);

    // Karakter (atlas varsa)
    const SDL_Rect* fr = m_atlas.frame(m_anim.index());
    if (fr) {
        m_r2d->drawTextureRegion(m_atlas.texture(), *fr, m_posX, m_posY, m_scale, m_rot);
    } else if (m_tex.sdl()) {
        m_r2d->drawTexture(m_tex, m_posX, m_posY, m_scale, m_rot);
    }

    m_r2d->present();
}

int Application::run() {
    if (!init()) { shutdown(); return -1; }

    bool running = true;
    Uint64 f = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();
    double fpsTimer=0.0; int fpsFrames=0; double currentFPS=0.0;

    while (running) {
        Uint64 now=SDL_GetPerformanceCounter(); Uint64 diff=now-last; last=now;
        double dt=(double)diff/(double)f; if (dt>0.1) dt=0.1;

        Input::beginFrame();
        SDL_PumpEvents();
        processEvents(running);
        Input::endFrame();

        update(dt);
        render();

        fpsTimer += dt; fpsFrames++;
        if (fpsTimer >= 0.5) {
            currentFPS = fpsFrames / fpsTimer; fpsFrames=0; fpsTimer=0.0;
            char title[256];
            std::snprintf(title, sizeof(title),
                "ErlikGameEngine | %.1f FPS%s | x=%.1f y=%.1f%s | anim=%.1ffps f=%d",
                currentFPS, m_paused ? " | PAUSED" : "",
                m_posX, m_posY,
                m_follow ? " | CAM=FOLLOW" : " | CAM=FIXED",
                m_anim.fps(), m_anim.index()
            );
            SDL_SetWindowTitle(m_window, title);
        }
    }
    shutdown();
    return 0;
}

} // namespace Erlik