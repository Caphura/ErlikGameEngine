#pragma once
#include <cstdint>
#include "Renderer2D.h"
#include "Texture.h"
#include "SpriteAtlas.h"
#include "Animator.h"
#include "Tilemap.h"
#include "Physics.h"

struct SDL_Window;
struct SDL_Renderer;

namespace Erlik {

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
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    int m_width  = 1280;
    int m_height = 720;

    // Render helpers
    Renderer2D* m_r2d = nullptr;
    Camera2D    m_cam;

    // World
    Tilemap     m_map;
    Player      m_player;
    PhysicsParams m_pp;

    // Visuals
    SpriteAtlas m_atlas;
    Animator    m_anim;

    // State
    bool  m_paused = false;
    bool  m_follow = true;
    double m_time=0.0;
};

} // namespace Erlik