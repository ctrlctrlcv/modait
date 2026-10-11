#include "CirclesOverlay.hpp"
#include "../OlkFPC.hpp"

using namespace geode::prelude;

namespace fbfc {

CirclesOverlay* CirclesOverlay::attach(PlayLayer* pl) {
    auto* r = new CirclesOverlay();
    if (!r || !r->CCNode::init()) { delete r; return nullptr; }
    r->autorelease();
    r->m_draw = CCDrawNode::create();
    r->m_labels = CCNode::create();
    r->addChild(r->m_draw); r->addChild(r->m_labels);
    pl->m_objectLayer->addChild(r, 1000);
    return r;
}

void CirclesOverlay::drawRing(CCDrawNode* d, CCPoint c, float r, float t, ccColor3B col) {
    d->drawCircle(c, r, {0.f, 0.f, 0.f, 0.f}, t, toF(col), 48);   // relleno transparente, solo borde
}
void CirclesOverlay::drawNumber(CCNode* parent, CCPoint p, int frames, ccColor3B col, float s) {
    auto* l = CCLabelBMFont::create(std::to_string(frames).c_str(), "bigFont.fnt");
    l->setScale(s); l->setColor(col); l->setPosition(p); parent->addChild(l);
}
void CirclesOverlay::drawMilliseconds(CCNode* parent, CCPoint p, double ms, ccColor3B col, float s) {
    auto* l = CCLabelBMFont::create(fmt::format("{:.1f}ms", ms).c_str(), "bigFont.fnt");
    l->setScale(s); l->setColor(col); l->setPosition(p); parent->addChild(l);
}

void CirclesOverlay::refresh() {
    auto* mod = Mod::get();
    bool on = mod->getSettingValue<bool>("counter-enabled") && mod->getSettingValue<bool>("circles-enabled");
    this->setVisible(on);
    auto& fpc = OlkFPC::get();
    if (!on || fpc.isAnalyzing() || m_rev == fpc.revision()) return;
    m_rev = fpc.revision();
    m_draw->clear(); m_labels->removeAllChildren();

    float size = static_cast<float>(mod->getSettingValue<double>("circles-size"));
    float thick = static_cast<float>(mod->getSettingValue<double>("circles-thickness"));
    float lscale = static_cast<float>(mod->getSettingValue<double>("circles-label-size"));
    int maxW = static_cast<int>(mod->getSettingValue<int64_t>("circles-max-window"));
    bool ms = mod->getSettingValue<bool>("circles-show-ms");

    for (auto const& r : fpc.results()) {
        if (!r.valid() || r.window > maxW) continue;
        CCPoint p{r.posX, r.posY};
        auto col = colorForWindow(r.window);
        drawRing(m_draw, p, 12.f * size, thick, col);
        CCPoint lp{p.x, p.y + 12.f * size + 8.f};
        if (ms) drawMilliseconds(m_labels, lp, r.windowMs, col, lscale);
        else drawNumber(m_labels, lp, r.window, col, lscale);
    }
}

}  // namespace fbfc
