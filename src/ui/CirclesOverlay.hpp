#pragma once
#include "../Types.hpp"

namespace fbfc {

// Anillos de precisión + etiquetas sobre las posiciones de cada click (espacio del m_objectLayer).
class CirclesOverlay : public cocos2d::CCNode {
public:
    static CirclesOverlay* attach(PlayLayer* pl);
    void refresh();                         // reconstruye si cambió la revisión del análisis
    static void drawRing(cocos2d::CCDrawNode* d, cocos2d::CCPoint c, float r, float thickness, cocos2d::ccColor3B col);
    static void drawNumber(cocos2d::CCNode* parent, cocos2d::CCPoint p, int frames, cocos2d::ccColor3B col, float scale);
    static void drawMilliseconds(cocos2d::CCNode* parent, cocos2d::CCPoint p, double ms, cocos2d::ccColor3B col, float scale);
private:
    uint64_t m_rev = UINT64_MAX;
    cocos2d::CCDrawNode* m_draw = nullptr;
    cocos2d::CCNode* m_labels = nullptr;
};

}  // namespace fbfc
