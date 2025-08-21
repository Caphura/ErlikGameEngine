#include "Physics.h"
#include <cmath>
#include <algorithm>

namespace Erlik {

static bool overlaps(float ax0,float ay0,float ax1,float ay1, float bx0,float by0,float bx1,float by1){
    return ax0 < bx1 && ax1 > bx0 && ay0 < by1 && ay1 > by0;
}

void integrate(Player& p, const Tilemap& map, const PhysicsParams& pp, float dt,
               bool moveLeft, bool moveRight, bool jump)
{
    const float tile = (float)map.tileSize();
    const float eps = 0.001f;

    // Hedef yatay hız ve basit hız karışımı
    float targetVX = 0.f;
    if(moveLeft)  targetVX -= pp.moveSpeed;
    if(moveRight) targetVX += pp.moveSpeed;
    float blend = p.onGround ? pp.accel : pp.accel * pp.airControl;
    p.vx = p.vx + (targetVX - p.vx) * std::clamp(blend * dt, 0.f, 1.f);

    // Yerçekimi
    p.vy += pp.gravity * dt;
    if(p.vy > pp.maxFall) p.vy = pp.maxFall;

    // Sıçrama
    if(jump && p.onGround){
        p.vy = pp.jumpVel;
        p.onGround = false;
    }

    // --- STEP X ---
    p.x += p.vx * dt;
    {
        float left = p.x - p.halfW;
        float right = p.x + p.halfW;
        float top = p.y - p.halfH;
        float bottom = p.y + p.halfH;

        int tx0 = (int)std::floor(left / tile);
        int tx1 = (int)std::floor(right / tile);
        int ty0 = (int)std::floor(top / tile);
        int ty1 = (int)std::floor(bottom / tile);

        // Sağ hareket -> sağdaki hücreleri kontrol et
        if(p.vx > 0.f){
            int col = (int)std::floor((p.x + p.halfW) / tile);
            for(int ty=ty0; ty<=ty1; ++ty){
                if(map.solidAtTile(col, ty)){
                    float tileLeft = col * tile;
                    p.x = tileLeft - p.halfW - eps;
                    p.vx = 0.f;
                    break;
                }
            }
        }
        // Sol hareket -> soldaki hücreleri kontrol et
        else if(p.vx < 0.f){
            int col = (int)std::floor((p.x - p.halfW) / tile);
            for(int ty=ty0; ty<=ty1; ++ty){
                if(map.solidAtTile(col, ty)){
                    float tileRight = (col+1) * tile;
                    p.x = tileRight + p.halfW + eps;
                    p.vx = 0.f;
                    break;
                }
            }
        }
    }

    // --- STEP Y ---
    p.y += p.vy * dt;
    p.onGround = false;
    {
        float left = p.x - p.halfW;
        float right = p.x + p.halfW;
        float top = p.y - p.halfH;
        float bottom = p.y + p.halfH;

        int tx0 = (int)std::floor(left / tile);
        int tx1 = (int)std::floor(right / tile);
        int ty0 = (int)std::floor(top / tile);
        int ty1 = (int)std::floor(bottom / tile);

        if(p.vy > 0.f){ // aşağı
            int row = (int)std::floor((p.y + p.halfH) / tile);
            for(int tx=tx0; tx<=tx1; ++tx){
                if(map.solidAtTile(tx, row)){
                    float tileTop = row * tile;
                    p.y = tileTop - p.halfH - eps;
                    p.vy = 0.f;
                    p.onGround = true;
                    break;
                }
            }
        } else if(p.vy < 0.f){ // yukarı çarpma
            int row = (int)std::floor((p.y - p.halfH) / tile);
            for(int tx=tx0; tx<=tx1; ++tx){
                if(map.solidAtTile(tx, row)){
                    float tileBottom = (row+1) * tile;
                    p.y = tileBottom + p.halfH + eps;
                    p.vy = 0.f;
                    break;
                }
            }
        }
    }
}

} // namespace Erlik