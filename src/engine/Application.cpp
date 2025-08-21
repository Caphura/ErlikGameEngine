#include "Application.h"
#include "Input.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>

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
    Input::init();

    // World setup
    if(!m_map.loadCSV("assets/level_aabb.csv")) std::fprintf(stderr,"[warn] assets/level_aabb.csv not found\n");
    if(!m_map.loadTileset(m_renderer, "assets/tileset32.png", 32)) std::fprintf(stderr,"[warn] assets/tileset32.png not found\n");

    // Player start near top-left
    m_player.x = 64.f; m_player.y = 64.f; m_player.vx=0.f; m_player.vy=0.f; m_player.onGround=false;

    // Visual sprite
    if(m_atlas.loadGrid(m_renderer, "assets/atlas8x1.png", 32,32,0,0)){
        m_anim.set(m_atlas.frameCount(), 10.f, true);
    }

    return true;
}

void Application::shutdown(){
    delete m_r2d; m_r2d=nullptr;
    Input::shutdown();
    if(m_renderer){ SDL_DestroyRenderer(m_renderer); m_renderer=nullptr; }
    if(m_window){ SDL_DestroyWindow(m_window); m_window=nullptr; }
    IMG_Quit(); SDL_Quit();
}

void Application::processEvents(bool& running){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if(e.type==SDL_QUIT) running=false;
    }
    if(Input::keyPressed(SDL_SCANCODE_ESCAPE)) running=false;
    if(Input::keyPressed(SDL_SCANCODE_SPACE))  m_paused = !m_paused;
    if(Input::keyPressed(SDL_SCANCODE_F))      m_follow = !m_follow;
    if(Input::keyPressed(SDL_SCANCODE_R)){
        m_player.x=64.f; m_player.y=64.f; m_player.vx=0.f; m_player.vy=0.f; m_player.onGround=false;
        m_cam = {};
    }
}

void Application::update(double dt){
    if(!m_paused){
        bool left  = Input::keyDown(SDL_SCANCODE_A) || Input::keyDown(SDL_SCANCODE_LEFT);
        bool right = Input::keyDown(SDL_SCANCODE_D) || Input::keyDown(SDL_SCANCODE_RIGHT);
        bool jump  = Input::keyPressed(SDL_SCANCODE_LSHIFT) || Input::keyPressed(SDL_SCANCODE_RSHIFT);

        integrate(m_player, m_map, m_pp, (float)dt, left, right, jump);
        m_anim.update(dt);
    }

    // Camera
    if(m_follow){
        m_cam.x = m_player.x - m_width  * 0.5f / m_cam.zoom;
        m_cam.y = m_player.y - m_height * 0.5f / m_cam.zoom;
    }

    m_time += dt;
}

void Application::render(){
    m_r2d->setCamera(m_cam);
    m_r2d->clear(12,12,16,255);
    m_r2d->drawGrid(64,50,50,60,255);

    // Draw tilemap
    m_map.draw(*m_r2d);

    // Draw player sprite (or fallback rect)
    const SDL_Rect* fr = m_atlas.frame(m_anim.index());
    if(fr){
        m_r2d->drawTextureRegion(m_atlas.texture(), *fr, m_player.x, m_player.y, 1.6f, 0.0f);
    }else{
        const int rectW=24, rectH=32;
        SDL_FRect r{ (m_player.x - m_cam.x)*m_cam.zoom - rectW*0.5f,
                     (m_player.y - m_cam.y)*m_cam.zoom - rectH*0.5f,
                     (float)rectW, (float)rectH };
        SDL_SetRenderDrawColor(m_renderer, 200,200,220,255);
        SDL_RenderFillRectF(m_renderer, &r);
    }

    // HUD title
    static double accum=0.0; static int frames=0; static double fps=0.0;
    accum += 1.0/60.0; frames++;
    if(frames>=30){
        char title[256];
        std::snprintf(title, sizeof(title),
            "ErlikGameEngine | player(%.1f,%.1f) v=(%.1f,%.1f) %s%s",
            m_player.x, m_player.y, m_player.vx, m_player.vy,
            m_paused? " | PAUSED":"",
            m_player.onGround? " | GROUND":""
        );
        SDL_SetWindowTitle(m_window, title);
        frames=0; accum=0.0;
    }

    m_r2d->present();
}

int Application::run(){
    if(!init()){ shutdown(); return -1; }
    bool running=true;
    Uint64 f=SDL_GetPerformanceFrequency(), last=SDL_GetPerformanceCounter();
    while(running){
        Uint64 now=SDL_GetPerformanceCounter(); Uint64 diff=now-last; last=now;
        double dt=(double)diff/(double)f; if(dt>0.1) dt=0.1;
        Input::beginFrame();
        SDL_PumpEvents();
        processEvents(running);
        Input::endFrame();
        update(dt);
        render();
    }
    shutdown();
    return 0;
}

} // namespace Erlik