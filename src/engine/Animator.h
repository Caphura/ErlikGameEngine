#pragma once
namespace Erlik {
class Animator{
public:
    void set(int count,float fps,bool loop=true){ m_count = count>0?count:1; m_fps=fps>0?fps:1.f; m_loop=loop; m_time=0.f; m_index=0; }
    void update(double dt){
        m_time += (float)dt; const float frameDur = 1.0f / m_fps;
        while(m_time>=frameDur){ m_time-=frameDur; m_index++; if(m_index>=m_count){ if(m_loop) m_index=0; else { m_index=m_count-1; break; } } }
    }
    int index() const { return m_index; }
    void setFPS(float f){ if(f>0) m_fps=f; }
    float fps() const { return m_fps; }
private:
    int m_count=1, m_index=0; float m_fps=8.f; bool m_loop=true; float m_time=0.f;
};
} // namespace Erlik