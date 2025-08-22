#include "Physics.h"
#include <cmath>
#include <algorithm>

namespace Erlik {

    static inline int tileFloor(float v, float tile) { return (int)std::floor(v / tile); }

    void integrate(Player& p, const Tilemap& map, const PhysicsParams& pp, float dt,
        bool moveLeft, bool moveRight, bool jumpPressed, bool jumpHeld, bool dropRequest)
    {
        const float tile = (float)map.tileSize();
        const float eps = 0.001f;

        // önceki konum
        p.prevX = p.x;
        p.prevY = p.y;

        // drop-through zamanlayıcısı
        p.dropTimer = std::max(0.f, p.dropTimer - dt);

        // Ayağın altında one-way var mı? (outer 'tile'ı kullan, İÇERİDE yeniden tanımlama YOK!)
        bool onewayUnder = false;
        {
            const float bottom = p.y + p.halfH + 0.1f;
            const int   rowUnder = (int)std::floor(bottom / tile);
            const int   tx0 = (int)std::floor((p.x - p.halfW + 1.0f) / tile);
            const int   tx1 = (int)std::floor((p.x + p.halfW - 1.0f) / tile);
            for (int tx = tx0; tx <= tx1; ++tx) {
                if (map.oneWayAtTile(tx, rowUnder)) { onewayUnder = true; break; }
            }
        }

        // Geçerli drop isteği: S+Jump ve zeminde ve altında one-way
        const bool doDrop = dropRequest && p.onGround && onewayUnder;
        if (doDrop) {
            p.dropTimer = pp.dropThroughTime;
            p.onGround = false;           // hemen serbest bırak
            p.jumpBufferTimer = 0.f;      // bu framede jump’ı iptal et
            if (p.vy < 30.f) p.vy = 30.f; // aşağı doğru minik itki
        }

        // --- JUMP/COYOTE sayaçları (önce sayaçları güncelle) ---
        // --- JUMP/COYOTE sayaçları ---
        if (!doDrop) {
            if (jumpPressed) p.jumpBufferTimer = pp.jumpBufferTime;
            else             p.jumpBufferTimer = std::max(0.f, p.jumpBufferTimer - dt);
        }
        else {
            // Drop anında jump tamamen bastırılsın
            p.jumpBufferTimer = 0.f;
            p.coyoteTimer = 0.f;
        }

        if (p.onGround)  p.coyoteTimer = pp.coyoteTime;
        else             p.coyoteTimer = std::max(0.f, p.coyoteTimer - dt);


        // --- Hedef yatay hız + sürtünme ---
        float targetVX = 0.f;
        if (moveLeft)  targetVX -= pp.moveSpeed;
        if (moveRight) targetVX += pp.moveSpeed;

        float blend = p.onGround ? pp.accel : pp.accel * pp.airControl;
        p.vx = p.vx + (targetVX - p.vx) * std::clamp(blend * dt, 0.f, 1.f);

        if (targetVX == 0.f) {
            float mu = p.onGround ? pp.frictionGround : pp.frictionAir;
            float factor = std::max(0.f, 1.f - mu * dt);
            p.vx *= factor;
            if (std::fabs(p.vx) < 0.01f) p.vx = 0.f;
        }

        // --- Zıplama (tek yer! drop varsa çalışmasın) ---
        if (!doDrop && p.jumpBufferTimer > 0.f && (p.onGround || p.coyoteTimer > 0.f)) {
            p.vy = pp.jumpVel;
            p.onGround = false;
            p.jumpBufferTimer = 0.f;
            p.coyoteTimer = 0.f;
        }

        // --- Yerçekimi + jump-cut ---
        p.vy += pp.gravity * dt;
        if (p.vy > pp.maxFall) p.vy = pp.maxFall;
        if (p.vy < 0.f && !jumpHeld) p.vy *= pp.jumpCutFactor;


        // -------- STEP X --------
        p.x += p.vx * dt;
        {
            float left = p.x - p.halfW;
            float right = p.x + p.halfW;
            float top = p.y - p.halfH + 1.0f;   // inset
            float bottom = p.y + p.halfH - 1.0f;   // inset

            if (p.vx > 0.f) {
                int col = tileFloor(right, tile);
                bool collided = false;

                // küçük basamaklara tırman
                for (int step = 0; step <= pp.stepMaxPixels; ++step) {
                    int ty0 = tileFloor((top - step), tile);
                    int ty1 = tileFloor((bottom - step), tile);
                    collided = false;
                    for (int ty = ty0; ty <= ty1; ++ty) {
                        if (map.solidAtTile(col, ty)) { collided = true; break; }
                    }
                    if (!collided) { p.y -= (float)step; top -= (float)step; bottom -= (float)step; break; }
                }

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
                    int ty0 = tileFloor((top - step), tile);
                    int ty1 = tileFloor((bottom - step), tile);
                    collided = false;
                    for (int ty = ty0; ty <= ty1; ++ty) {
                        if (map.solidAtTile(col, ty)) { collided = true; break; }
                    }
                    if (!collided) { p.y -= (float)step; top -= (float)step; bottom -= (float)step; break; }
                }

                if (collided) {
                    float tileRight = (col + 1) * tile;
                    p.x = tileRight + p.halfW + eps;
                    p.vx = 0.f;
                }
            }
        }

        // -------- STEP Y --------
        p.y += p.vy * dt;
        p.onGround = false;
        {
            float left = p.x - p.halfW + 1.0f; // inset
            float right = p.x + p.halfW - 1.0f; // inset
            float top = p.y - p.halfH;
            float bottom = p.y + p.halfH;

            int tx0 = tileFloor(left, tile);
            int tx1 = tileFloor(right, tile);

            if (p.vy > 0.f) { // aşağı
                int row = tileFloor(bottom, tile);
                for (int tx = tx0; tx <= tx1; ++tx) {
                    bool solid = map.solidAtTile(tx, row);
                    bool oneway = map.oneWayAtTile(tx, row);
                    float tileTop = row * tile;
                    float prevBottom = p.prevY + p.halfH;

                    const bool onewayCounts = oneway && (p.dropTimer <= 0.f) && (prevBottom <= tileTop);
                    if (solid || onewayCounts) {
                        p.y = tileTop - p.halfH - eps;
                        p.vy = 0.f;
                        p.onGround = true;
                        break;
                    }
                }

                // ground snap
                if (!p.onGround) {
                    float snap = pp.groundSnapDist;
                    int rowSnap = tileFloor(bottom + snap, tile);
                    for (int tx = tx0; tx <= tx1; ++tx) {
                        bool solid = map.solidAtTile(tx, rowSnap);
                        bool oneway = map.oneWayAtTile(tx, rowSnap);
                        float tileTop = rowSnap * tile;
                        float prevBottom = p.prevY + p.halfH;

                        const bool onewayCounts = oneway && (p.dropTimer <= 0.f) && (prevBottom <= tileTop);
                        if (solid || onewayCounts) {
                            float dist = tileTop - bottom;
                            if (dist >= 0.f && dist <= snap) {
                                p.y += dist; p.vy = 0.f; p.onGround = true;
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
