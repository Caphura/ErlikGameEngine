#pragma once
#include <cstdint>

namespace Erlik {

    class Tilemap;


    // Oyuncu (fizik çekirdeği bu alanları kullanıyor)
    struct Player {
        // Konum & hız
        float x = 0.f, y = 0.f;
        float vx = 0.f, vy = 0.f;

        // Önceki frame konumu (çarpışma ve oneway kontrolü için)
        float prevX = 0.f, prevY = 0.f;

        // Boyut (yarım genişlik / yarım yükseklik)
        float halfW = 12.f;   // 24 px genişlik
        float halfH = 16.f;   // 32 px yükseklik

        bool  onGround = false;

        // Zamanlayıcılar
        float dropTimer = 0.f;   // one-way'den aşağı bırakma süresi
        float coyoteTimer = 0.f;   // coyote time (yer toleransı)
        float jumpBufferTimer = 0.f;   // jump buffer (erken basma toleransı)
    };

    // Fizik parametreleri (ayarlar)
    struct PhysicsParams {
        // Yatay hareket
        float moveSpeed = 120.f;  // px/s
        float accel = 10.f;   // ivmelenme karışım katsayısı (büyüdükçe daha hızlı oturur)
        float frictionGround = 8.f;    // yerde sürtünme
        float frictionAir = 1.5f;   // havada sürtünme
        float airControl = 0.5f;   // havada accel çarpanı (0..1)

        // Zıplama & yerçekimi
        float jumpVel = -660.f; // NEGATİF = yukarı itiş
        float gravity = 850.f;  // +aşağı
        float maxFall = 420.f;  // terminal düşüş hızı
        float jumpCutFactor = 0.5f;   // Space erken bırakılırsa yukarı hızın çarpanı

        // Küçük basamaklara “tırmanma” ve yere yapışma
        int   stepMaxPixels = 4;      // en çok şu kadar piksel yukarı tırman
        float groundSnapDist = 4.f;    // yere yakınsa çektir

        // One-way platformdan aşağı bırakma (S+Space)
        float dropThroughTime = 0.18f;

        // Oyun hissi 2.0
        float coyoteTime = 0.12f;  // 120 ms
        float jumpBufferTime = 0.12f;  // 120 ms
    };

    // Physics.cpp’deki imza ile birebir aynı olmalı
    void integrate(
        Player& p,
        const class Tilemap& map,
        const PhysicsParams& pp,
        float dt,
        bool moveLeft, bool moveRight,
        bool jumpPressed, bool jumpHeld,
        bool dropRequest
    );

} // namespace Erlik
