#pragma once
#include "Tilemap.h"

namespace Erlik {

struct Player {
    float x=0.f, y=0.f;
    float vx=0.f, vy=0.f;
    bool onGround=false;
    float halfW=12.f, halfH=16.f; // 24x32 kapsayan kutu
    float prevX = 0.f, prevY = 0.f;
    float dropTimer = 0.f;        // one-way'i geçici olarak yok saymak için
    float coyoteTimer = 0.f, jumpBufferTimer = 0.f;

};

struct PhysicsParams {
    float gravity = 1800.f;
    float moveSpeed = 260.f;
    float airControl = 0.6f;
    float accel = 12.f;
    float maxFall = 900.f;
    float jumpVel = -900.f;
    float dropThroughTime = 0.18f; // Down+Jump ile aşağı sarkınca ne kadar süre one-way yok sayılacak

    // Önceden eklediklerimiz:
    float coyoteTime = 0.10f;
    float jumpBufferTime = 0.12f;
    float groundSnapDist = 3.0f;
    int   stepMaxPixels = 8;

    // YENİ: sürtünme ve kısa zıplama
    float frictionGround = 20.f; // px/s sürtünme katsayısı
    float frictionAir = 1.f; // havada hafif sönüm
    float jumpCutFactor = 0.5f; // tuş bırakılınca yukarı hızla çarp (0.5 = %50)
};

// İMZA: jumpPressed + jumpHeld ++ dropRequest
void integrate(Player& p, const Tilemap& map, const PhysicsParams& pp, float dt,
    bool moveLeft, bool moveRight, bool jumpPressed, bool jumpHeld, bool dropRequest);


} // namespace Erlik