#pragma once
#include <Geode/Geode.hpp>
#include "../OlkFPC.hpp"

using namespace geode::prelude;

class ToolsOverlay : public cocos2d::CCNode {
public:
    static ToolsOverlay* create();
    bool init() override;
    void update(float dt) override;

    void setOverlayVisible(bool visible) { m_visible = visible; }

private:
    bool m_visible = true;

    // Hitboxes
    void drawHitboxes();
    void drawHitboxTrail();

    // Trayectoria
    void drawTrajectory();

    // Frame stepper
    void drawFrameStepper();

    // Input display
    void drawInputDisplay();

    // Nodos para limpieza
    std::vector<cocos2d::CCDrawNode*> m_drawNodes;
    std::vector<cocos2d::CCLabelBMFont*> m_labels;

    // Trail de hitboxes
    struct TrailPoint {
        cocos2d::CCPoint pos;
        float time;
    };
    std::vector<TrailPoint> m_trail;

    // Frame stepper
    int m_steppedFrame = 0;
    bool m_isStepping = false;

    // Input display
    std::vector<std::string> m_inputHistory;

    void clearNodes();
};