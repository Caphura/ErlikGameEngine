#pragma once
#include <SDL.h>
#include <string>
struct SDL_Renderer; struct SDL_Texture;
namespace Erlik{
class Texture{
public:
 Texture()=default; ~Texture(); bool loadFromFile(SDL_Renderer*, const std::string& path);
 void destroy(); SDL_Texture* sdl() const {return m_tex;} int width()const{return m_w;} int height()const{return m_h;}
private: SDL_Texture* m_tex=nullptr; int m_w=0; int m_h=0; };
}