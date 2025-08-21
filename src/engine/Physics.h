#pragma once
#include "Tilemap.h"

namespace Erlik {

struct Player {
    float x=0.f, y=0.f;
    float vx=0.f, vy=0.f;
    bool onGround=false;
    float halfW=12.f, halfH=16.f; // 24x32 kapsayan kutu
};

struct PhysicsParams {
    float gravity = 1800.f;     // px/s^2
    float moveSpeed = 260.f;    // hedef yatay hız
    float airControl = 0.6f;    // havada hız karışımı
    float accel = 12.f;         // yatay hız blend katsayısı
    float maxFall = 900.f;      // terminal hız
    float jumpVel = -900.f;     // sıçrama hızı
};

// Tile bazlı AABB çözümü (önce X sonra Y)
void integrate(Player& p, const Tilemap& map, const PhysicsParams& pp, float dt,
               bool moveLeft, bool moveRight, bool jump);

} // namespace Erlik