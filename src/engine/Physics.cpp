// Physics.cpp
#include "Physics.h"
#include <cmath>
#include <algorithm>

namespace Erlik {

    static inline int tileFloor(float v, float tile) { return (int)std::floor(v / tile); }
    static inline int tileIdxX(float x, float halfW, float tile) { return tileFloor(x + halfW, tile); }
    static inline int tileIdxXNeg(float x, float halfW, float tile) { return tileFloor(x - halfW, tile); }

    void integrate(Player& p, const Tilemap& map, const PhysicsParams& pp, float dt,
        bool moveLeft, bool moveRight, bool jumpPressed, bool jumpHeld)
    {
        const float tile = (float)map.tileSize();
        const float eps = 0.001f;

        // --- Sayaçlar (coyote + buffer) ---
        if (jumpPressed) p.jumpBufferTimer = pp.jumpBufferTime;
        else             p.jumpBufferTimer = std::max(0.f, p.jumpBufferTimer - dt);

        if (p.onGround)  p.coyoteTimer = pp.coyoteTime;
        else             p.coyoteTimer = std::max(0.f, p.coyoteTimer - dt);

        // --- Hedef yatay hız (zeminde tam, havada kısmi kontrol) ---
        float targetVX = 0.f;
        if (moveLeft)  targetVX -= pp.moveSpeed;
        if (moveRight) targetVX += pp.moveSpeed;
        float blend = p.onGround ? pp.accel : pp.accel * pp.airControl;
        p.vx = p.vx + (targetVX - p.vx) * std::clamp(blend * dt, 0.f, 1.f);

        // Hedef yatay hız belirlendikten ve p.vx blend edildikten sonra:
        if (targetVX == 0.f) {
            float mu = p.onGround ? pp.frictionGround : pp.frictionAir;
            float factor = std::max(0.f, 1.f - mu * dt);
            p.vx *= factor;
            if (std::fabs(p.vx) < 0.01f) p.vx = 0.f;
        }

        // --- Zıplama: coyote veya zemindeyken, buffer doluyken tetikle ---
        if (p.jumpBufferTimer > 0.f && (p.onGround || p.coyoteTimer > 0.f)) {
            p.vy = pp.jumpVel;
            p.onGround = false;
            p.jumpBufferTimer = 0.f;
            p.coyoteTimer = 0.f;
        }

        // --- Yerçekimi ---
        p.vy += pp.gravity * dt;
        if (p.vy > pp.maxFall) p.vy = pp.maxFall;

        // Jump-cut: tuş bırakıldıysa yukarı hızı kırp
        if (p.vy < 0.f && !jumpHeld) {
            p.vy *= pp.jumpCutFactor;
        }


        // ===================== STEP X =====================
        p.x += p.vx * dt;

        {
            // İnset: köşe takılmasını azaltmak için üst/alt kenarı 1px içeri al
            float left = p.x - p.halfW;
            float right = p.x + p.halfW;
            float top = p.y - p.halfH + 1.0f;
            float bottom = p.y + p.halfH - 1.0f;

            //int ty0 = tileFloor(top, tile);
            //int ty1 = tileFloor(bottom, tile);

            if (p.vx > 0.f) {
                int col = tileFloor(right, tile);
                bool collided = false;

                // Step-up denemesi (küçük basamaklar)
                for (int step = 0; step <= pp.stepMaxPixels; ++step) {
                    int testTy0 = tileFloor((top - step), tile);
                    int testTy1 = tileFloor((bottom - step), tile);
                    collided = false;
                    for (int ty = testTy0; ty <= testTy1; ++ty) {
                        if (map.solidAtTile(col, ty)) { collided = true; break; }
                    }
                    if (!collided) {
                        // Basamağa tırmandık
                        p.y -= (float)step;
                        top -= (float)step;
                        bottom -= (float)step;
                        break;
                    }
                }

                // Hâlâ çarpışıyorsa yatay çöz
                if (collided) {
                    float tileLeft = col * tile;
                    p.x = tileLeft - p.halfW - eps;
                    p.vx = 0.f;
                }
            }
            else if (p.vx < 0.f) {
                int col = tileFloor(left, tile);
                bool collided = false;

                for (int step = 0; step <= pp.stepMaxPixels; ++step) {
                    int testTy0 = tileFloor((top - step), tile);
                    int testTy1 = tileFloor((bottom - step), tile);
                    collided = false;
                    for (int ty = testTy0; ty <= testTy1; ++ty) {
                        if (map.solidAtTile(col, ty)) { collided = true; break; }
                    }
                    if (!collided) {
                        p.y -= (float)step;
                        top -= (float)step;
                        bottom -= (float)step;
                        break;
                    }
                }

                if (collided) {
                    float tileRight = (col + 1) * tile;
                    p.x = tileRight + p.halfW + eps;
                    p.vx = 0.f;
                }
            }
        }

        // ===================== STEP Y =====================
        p.y += p.vy * dt;
        p.onGround = false;

        {
            // İnset: sol/sağ kenarı 1px içeri al
            float left = p.x - p.halfW + 1.0f;
            float right = p.x + p.halfW - 1.0f;
            float top = p.y - p.halfH;
            float bottom = p.y + p.halfH;

            int tx0 = tileFloor(left, tile);
            int tx1 = tileFloor(right, tile);

            if (p.vy > 0.f) { // aşağı düşüyor
                int row = tileFloor(bottom, tile);
                for (int tx = tx0; tx <= tx1; ++tx) {
                    if (map.solidAtTile(tx, row)) {
                        float tileTop = row * tile;
                        p.y = tileTop - p.halfH - eps;
                        p.vy = 0.f;
                        p.onGround = true;
                        break;
                    }
                }

                // Ground snap: çok az mesafe kaldıysa aşağı yapıştır
                if (!p.onGround) {
                    float snap = pp.groundSnapDist;
                    int rowSnap = tileFloor(bottom + snap, tile);
                    for (int tx = tx0; tx <= tx1; ++tx) {
                        if (map.solidAtTile(tx, rowSnap)) {
                            float tileTop = rowSnap * tile;
                            float dist = tileTop - bottom;
                            if (dist >= 0.f && dist <= snap) {
                                p.y += dist;          // aşağı indir
                                p.vy = 0.f;
                                p.onGround = true;
                            }
                            break;
                        }
                    }
                }
            }
            else if (p.vy < 0.f) { // yukarı
                int row = tileFloor(top, tile);
                for (int tx = tx0; tx <= tx1; ++tx) {
                    if (map.solidAtTile(tx, row)) {
                        float tileBottom = (row + 1) * tile;
                        p.y = tileBottom + p.halfH + eps;
                        p.vy = 0.f;
                        break;
                    }
                }
            }
        }
    }

} // namespace Erlik
