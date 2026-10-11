#include "ToolsOverlay.hpp"
#include "../OlkFPC.hpp"

using namespace geode::prelude;

namespace fbfc {

ToolsOverlay* ToolsOverlay::attach(PlayLayer* pl) {
    auto* r = new ToolsOverlay();
    if (!r || !r->CCNode::init()) { delete r; return nullptr; }
    r->autorelease();
    r->m_world = CCDrawNode::create();
    pl->m_objectLayer->addChild(r->m_world, 1001);
    r->m_hud = CCDrawNode::create();
    r->m_hud->setPosition({10.f, 10.f});
    pl->addChild(r->m_hud, 1001);
    pl->addChild(r, 0);
    return r;
}

void ToolsOverlay::drawRect(CCRect const& r, ccColor4F border, ccColor4F fill) {
    CCPoint v[4] = {{r.getMinX(), r.getMinY()}, {r.getMaxX(), r.getMinY()}, {r.getMaxX(), r.getMaxY()}, {r.getMinX(), r.getMaxY()}};
    m_world->drawPolygon(v, 4, fill, 0.6f, border);
}

void ToolsOverlay::update(PlayLayer* pl) {
    auto* mod = Mod::get();
    m_world->clear(); m_hud->clear();
    bool hb = mod->getSettingValue<bool>("tools-hitboxes");
    bool trail = mod->getSettingValue<bool>("tools-trail");
    auto frame = OlkFPC::get().currentFrame();

    PlayerObject* ps[2] = {pl->m_player1, pl->m_player2};
    if (frame != m_lastFrame) {
        m_lastFrame = frame;
        if (trail && ps[0]) {
            m_trail.push_back(ps[0]->getObjectRect());
            size_t cap = static_cast<size_t>(mod->getSettingValue<int64_t>("tools-trail-length"));
            while (m_trail.size() > cap) m_trail.pop_front();
        }
        if (!trail) m_trail.clear();
    }
    if (trail) for (auto& r : m_trail) drawRect(r, {1.f, 1.f, 0.f, 0.5f}, {0.f, 0.f, 0.f, 0.f});
    if (hb) for (auto* p : ps) {
        if (!p || !p->isVisible()) continue;
        drawRect(p->getObjectRect(), {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 0.15f});
        drawRect(p->getObjectRect(0.25f, 0.25f), {0.f, 0.6f, 1.f, 1.f}, {0.f, 0.6f, 1.f, 0.25f});
    }
    if (mod->getSettingValue<bool>("tools-trajectory") && frame % 10 == 0) predict(pl, 90);
    for (size_t i = 1; i < m_path.size(); ++i) m_world->drawSegment(m_path[i - 1], m_path[i], 0.8f, {0.f, 1.f, 0.4f, 0.8f});

    if (mod->getSettingValue<bool>("tools-input-display")) {
        for (int p = 0; p < 2; ++p) {
            bool down = OlkFPC::get().held(p == 1);
            CCPoint a{p * 26.f, 0.f}, b{p * 26.f + 22.f, 22.f};
            m_hud->drawRect(a, b, down ? ccColor4F{0.2f, 1.f, 0.3f, 0.9f} : ccColor4F{0.f, 0.f, 0.f, 0.4f}, 1.f, {1.f, 1.f, 1.f, 0.8f});
        }
    }
}

// EXPERIMENTAL: avanza el PlayLayer real N frames sobre un checkpoint y lo restaura.
void ToolsOverlay::predict(PlayLayer* pl, int frames) {
    if (!pl->m_player1 || pl->m_player1->m_isDead) return;
    auto& fpc = OlkFPC::get();
    fpc.pushSim();                                    // los hooks ignoran esta simulación
    Ref<CheckpointObject> cp = pl->createCheckpoint();
    m_path.clear();
    float dt = 1.f / static_cast<float>(Mod::get()->getSettingValue<int64_t>("macro-fps"));
    for (int i = 0; i < frames && !pl->m_player1->m_isDead; ++i) {
        pl->update(dt);
        m_path.push_back(pl->m_player1->getPosition());
    }
    pl->loadFromCheckpoint(cp);
    fpc.popSim();
}

void ToolsOverlay::stepFrame() {
    auto* pl = PlayLayer::get();
    if (!pl) return;
    pl->update(1.f / static_cast<float>(Mod::get()->getSettingValue<int64_t>("macro-fps")));
}

}  // namespace fbfc
