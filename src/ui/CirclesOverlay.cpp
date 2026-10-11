#include "CirclesOverlay.hpp"

CirclesOverlay* CirclesOverlay::create() {
    auto ret = new CirclesOverlay();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CirclesOverlay::init() {
    if (!CCNode::init()) return false;
    this->scheduleUpdate();
    return true;
}

void CirclesOverlay::update(float dt) {
    for (auto node : m_circleNodes) node->removeFromParent();
    for (auto label : m_numberLabels) label->removeFromParent();
    m_circleNodes.clear();
    m_numberLabels.clear();

    auto& olk = *OlkFPC::get();
    if (!olk.m_isPlaying || olk.m_clicks.empty()) return;

    auto playLayer = PlayLayer::get();
    if (!playLayer) return;

    int currentFrame = static_cast<int>(playLayer->m_gameState.m_levelTime * 240.0f);
    if (currentFrame < 0) return;

    int windowStart = std::max(0, currentFrame - 30);
    int windowEnd = std::min(static_cast<int>(olk.m_clicks.size()) - 1, currentFrame + 30);

    auto mod = Mod::get();
    float circleSize = static_cast<float>(mod->getSettingValue<double>("circle-size")) / 100.0f;
    float numberSize = static_cast<float>(mod->getSettingValue<double>("number-size")) / 100.0f;
    bool showMs = mod->getSettingValue<bool>("ms-under-number");
    bool numberBeside = mod->getSettingValue<bool>("number-beside-ring");

    for (int i = windowStart; i <= windowEnd; ++i) {
        auto& click = olk.m_clicks[i];

        cocos2d::ccColor4B color;
        if (click.frameWindow == 0)      color = {255, 100, 100, 255};
        else if (click.frameWindow == 1) color = {255, 165, 0, 255};
        else if (click.frameWindow == 2) color = {255, 255, 100, 255};
        else if (click.frameWindow == 3) color = {255, 255, 255, 255};
        else if (click.frameWindow <= 4) color = {150, 255, 150, 255};
        else if (click.frameWindow <= 6) color = {100, 255, 255, 255};
        else if (click.frameWindow <= 8) color = {100, 200, 255, 255};
        else                              color = {200, 150, 255, 255};

        float screenX = 200 + i * 35.0f;
        float screenY = 100;

        drawRing({screenX, screenY}, 15.0f * circleSize, color, 3.0f);

        std::string numStr = (click.frameWindow >= 9) ? "9+" : std::to_string(click.frameWindow);
        auto numLabel = CCLabelBMFont::create(numStr.c_str(), "bigFont.fnt");
        numLabel->setScale(0.4f * numberSize);
        numLabel->setColor({color.r, color.g, color.b});
        numLabel->setPosition(numberBeside ? screenX + 25.0f * circleSize : screenX, screenY);
        m_numberLabels.push_back(numLabel);
        this->addChild(numLabel);

        if (showMs) {
            float ms = (click.frameWindow * 1000.0f) / 240.0f;
            drawMilliseconds({screenX, screenY - 20.0f * circleSize}, ms);
        }
    }
}

void CirclesOverlay::drawRing(const cocos2d::CCPoint& pos, float radius,
                               const cocos2d::ccColor4B& color, float thickness) {
    auto drawNode = CCDrawNode::create();

    // ✅ CONVERSIÓN CORREGIDA: ccColor4B → ccColor4F
    cocos2d::ccColor4F colorF = ccc4FFromccc4B(color);

    const int segments = 32;
    float angleStep = 2.0f * M_PI / segments;

    for (int i = 0; i < segments; ++i) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;

        CCPoint p1 = {pos.x + radius * cosf(a1), pos.y + radius * sinf(a1)};
        CCPoint p2 = {pos.x + radius * cosf(a2), pos.y + radius * sinf(a2)};

        drawNode->drawSegment(p1, p2, thickness, colorF);   // ← usa colorF
    }

    m_circleNodes.push_back(drawNode);
    this->addChild(drawNode);
}

void CirclesOverlay::drawNumber(const cocos2d::CCPoint& pos, int number,
                                 const cocos2d::ccColor4B& color) {
    auto label = CCLabelBMFont::create(std::to_string(number).c_str(), "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition(pos);
    label->setColor({color.r, color.g, color.b});
    m_numberLabels.push_back(label);
    this->addChild(label);
}

void CirclesOverlay::drawMilliseconds(const cocos2d::CCPoint& pos, float ms) {
    auto msLabel = CCLabelBMFont::create(
        fmt::format("{:.1f}ms", ms).c_str(), "chatFont.fnt");
    msLabel->setScale(0.35f);
    msLabel->setPosition(pos);
    msLabel->setColor({150, 150, 180});
    m_numberLabels.push_back(msLabel);
    this->addChild(msLabel);
}
