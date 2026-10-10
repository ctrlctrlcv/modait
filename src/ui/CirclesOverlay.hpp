// CirclesOverlay.hpp
#pragma once
#include <Geode/Geode.hpp>
#include "OlkFPC.hpp"

class CirclesOverlay : public cocos2d::CCNode {
public:
    static CirclesOverlay* create();
    bool init() override;
    void update(float dt) override;
    void drawRing(const cocos2d::CCPoint& pos, float radius, const cocos2d::ccColor4B& color, float thickness);
    void drawNumber(const cocos2d::CCPoint& pos, int number, const cocos2d::ccColor4B& color);
    void drawMilliseconds(const cocos2d::CCPoint& pos, float ms);

private:
    std::vector<cocos2d::CCDrawNode*> m_circleNodes;
    std::vector<cocos2d::CCLabelBMFont*> m_numberLabels;
};