#pragma once
#include "../Types.hpp"
#include <deque>

namespace fbfc {

// Hitboxes, trail, trayectoria predicha, frame stepper e input-display.
class ToolsOverlay : public cocos2d::CCNode {
public:
    static ToolsOverlay* attach(PlayLayer* pl);
    void update(PlayLayer* pl);
    static void stepFrame();                 // avanza un frame con el juego pausado
private:
    void drawRect(cocos2d::CCRect const& r, cocos2d::ccColor4F border, cocos2d::ccColor4F fill);
    void predict(PlayLayer* pl, int frames);

    cocos2d::CCDrawNode* m_world = nullptr;
    cocos2d::CCDrawNode* m_hud = nullptr;
    std::deque<cocos2d::CCRect> m_trail;
    std::vector<cocos2d::CCPoint> m_path;
    int64_t m_lastFrame = -1;
};

}  // namespace fbfc
