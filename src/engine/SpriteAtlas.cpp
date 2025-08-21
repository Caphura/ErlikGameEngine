#include "SpriteAtlas.h"
#include <SDL.h>
namespace Erlik{
bool SpriteAtlas::loadGrid(SDL_Renderer* r,const std::string& path,int frameW,int frameH,int margin,int spacing){
 if(!m_tex.loadFromFile(r,path)) return false;
 m_frames.clear();
 int texW=m_tex.width(), texH=m_tex.height();
 for(int y=margin; y+frameH<=texH; y+=frameH+spacing){
  for(int x=margin; x+frameW<=texW; x+=frameW+spacing){
   SDL_Rect rc{ x,y,frameW,frameH }; m_frames.push_back(rc);
  }
 }
 return !m_frames.empty();
}
const SDL_Rect* SpriteAtlas::frame(int i) const{
 if(i<0 || i>=(int)m_frames.size()) return nullptr; return &m_frames[i];
}
}