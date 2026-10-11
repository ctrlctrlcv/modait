#include "SettingsUI.hpp"
#include "FPSSettingsUI.hpp"
#include "SettingWidgets.hpp"

using namespace geode::prelude;

namespace fbfc {

namespace {
enum class K { Toggle, SliderF, SliderI, Text, Cycle, FPS };
struct Row { K kind; char const* key; char const* label; double mn = 0, mx = 1; std::vector<std::string> opts = {}; };

std::vector<std::pair<char const*, std::vector<Row>>> const& tabs() {
    static std::vector<std::pair<char const*, std::vector<Row>>> t = {
        {"Counter", {
            {K::Toggle, "counter-enabled", "Counter enabled"},
            {K::Toggle, "rows-enabled", "Rows overlay"},
            {K::Cycle, "rows-mode", "Rows mode", 0, 0, {"Short", "Normal", "Custom"}},
            {K::Text, "rows-custom", "Rows custom (0-3,5,9+)"},
            {K::SliderF, "rows-size", "Rows size", 0.2, 1.5}}},
        {"Circles", {
            {K::Toggle, "circles-enabled", "Circles"},
            {K::Toggle, "circles-show-ms", "Show milliseconds"},
            {K::SliderF, "circles-size", "Circle size", 0.3, 3.0},
            {K::SliderF, "circles-thickness", "Thickness", 0.5, 8.0},
            {K::SliderF, "circles-label-size", "Label size", 0.1, 1.0},
            {K::SliderI, "circles-max-window", "Max window", 0, 64}}},
        {"Sounds", {
            {K::Toggle, "sound-enabled", "Sounds"},
            {K::Toggle, "sound-pitch", "Variable pitch"},
            {K::Toggle, "sound-local", "Use bing.mp3"},
            {K::Cycle, "sound-timing", "Timing", 0, 0, {"On Click", "Before Click", "After Click", "Perfect Frame"}},
            {K::SliderF, "sound-volume", "Volume", 0.0, 2.0}}},
        {"Scan", {
            {K::FPS, "", "Macro / Scan FPS"},
            {K::SliderI, "scan-max-shift", "Max shift", 1, 64},
            {K::SliderI, "scan-horizon", "Horizon", 10, 2400},
            {K::SliderI, "scan-budget-ms", "Budget ms", 1, 50},
            {K::Toggle, "scan-auto", "Auto-scan on complete"}}},
        {"Recording", {
            {K::Toggle, "record-auto", "Auto record inputs"},
            {K::Cycle, "render-preset", "Render preset", 0, 0, {"720p30", "720p60", "1080p30", "1080p60", "1440p60", "4K30", "4K60"}},
            {K::SliderI, "render-crf", "CRF", 0, 51},
            {K::Text, "ffmpeg-path", "FFmpeg path"}}},
        {"Tools", {
            {K::Toggle, "tools-hitboxes", "Hitboxes"},
            {K::Toggle, "tools-trail", "Hitbox trail"},
            {K::SliderI, "tools-trail-length", "Trail length", 10, 2000},
            {K::Toggle, "tools-trajectory", "Trajectory (exp.)"},
            {K::Toggle, "tools-input-display", "Input display"},
            {K::Toggle, "tools-stepper", "Frame stepper"}}},
    };
    return t;
}
}  // namespace

SettingsUI* SettingsUI::create(CCSize size) {
    auto* r = new SettingsUI();
    if (r->init(size)) { r->autorelease(); return r; }
    delete r; return nullptr;
}

bool SettingsUI::init(CCSize size) {
    if (!CCNode::init()) return false;
    m_size = size;
    this->setContentSize(size);
    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition({0.f, 0.f});
    this->addChild(m_tabMenu);
    for (int i = 0; i < static_cast<int>(tabs().size()); ++i) {
        auto* spr = ButtonSprite::create(tabs()[i].first, 60, true, "bigFont.fnt", "GJ_button_04.png", 22.f, 0.5f);
        auto* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SettingsUI::onTab));
        btn->setTag(i);
        btn->setPosition({40.f + i * 64.f, size.height - 12.f});
        m_tabMenu->addChild(btn);
    }
    selectTab(0);
    return true;
}

void SettingsUI::onTab(CCObject* s) { selectTab(static_cast<CCNode*>(s)->getTag()); }

void SettingsUI::selectTab(int i) {
    m_tab = i;
    if (m_page) m_page->removeFromParent();
    m_page = CCNode::create();
    this->addChild(m_page);
    auto* menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    m_page->addChild(menu);

    float y = m_size.height - 48.f;
    for (auto const& row : tabs()[i].second) {
        auto* mod = Mod::get();
        float x = 14.f;
        switch (row.kind) {
        case K::Toggle: {
            auto* t = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SettingsUI::onToggle), 0.5f);
            t->setID(row.key);
            t->toggle(mod->getSettingValue<bool>(row.key));
            t->setPosition({x + 10.f, y});
            menu->addChild(t);
            auto* l = CCLabelBMFont::create(row.label, "bigFont.fnt");
            l->setScale(0.38f); l->setAnchorPoint({0.f, 0.5f}); l->setPosition({x + 30.f, y});
            m_page->addChild(l);
        } break;
        case K::SliderF: case K::SliderI: {
            auto* s = SettingSlider::create(row.key, row.label, row.mn, row.mx, false, row.kind == K::SliderI);
            s->setPosition({x, y - 8.f});
            m_page->addChild(s);
        } break;
        case K::Text: {
            auto* l = CCLabelBMFont::create(row.label, "bigFont.fnt");
            l->setScale(0.34f); l->setAnchorPoint({0.f, 0.5f}); l->setPosition({x, y});
            m_page->addChild(l);
            auto* in = TextInput::create(140.f, row.label, "bigFont.fnt");
            in->setScale(0.7f);
            in->setPosition({m_size.width - 110.f, y});
            in->setString(mod->getSettingValue<std::string>(row.key));
            std::string key = row.key;
            in->setCallback([key](std::string const& v) { (void)Mod::get()->setSettingValue<std::string>(key, v); });
            m_page->addChild(in);
        } break;
        case K::Cycle: {
            auto* spr = ButtonSprite::create(mod->getSettingValue<std::string>(row.key).c_str(), 110, true, "bigFont.fnt", "GJ_button_04.png", 24.f, 0.5f);
            auto* b = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SettingsUI::onCycle));
            b->setID(row.key);
            b->setPosition({m_size.width - 80.f, y});
            menu->addChild(b);
            auto* l = CCLabelBMFont::create(row.label, "bigFont.fnt");
            l->setScale(0.38f); l->setAnchorPoint({0.f, 0.5f}); l->setPosition({x, y});
            m_page->addChild(l);
        } break;
        case K::FPS: {
            auto* spr = ButtonSprite::create("Open FPS settings", 160, true, "bigFont.fnt", "GJ_button_01.png", 26.f, 0.5f);
            auto* b = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SettingsUI::onFPS));
            b->setPosition({m_size.width / 2.f, y});
            menu->addChild(b);
        } break;
        }
        y -= 34.f;
    }
}

void SettingsUI::onToggle(CCObject* s) {
    auto* t = static_cast<CCMenuItemToggler*>(s);
    (void)Mod::get()->setSettingValue<bool>(t->getID(), !t->isToggled());   // el toggler cambia después del callback
}

void SettingsUI::onCycle(CCObject* s) {
    auto* b = static_cast<CCMenuItemSpriteExtra*>(s);
    std::string key = b->getID();
    for (auto const& row : tabs()[m_tab].second) {
        if (key != row.key) continue;
        auto cur = Mod::get()->getSettingValue<std::string>(key);
        size_t i = std::ranges::find(row.opts, cur) - row.opts.begin();
        (void)Mod::get()->setSettingValue<std::string>(key, row.opts[(i + 1) % row.opts.size()]);
        break;
    }
    selectTab(m_tab);
}

void SettingsUI::onFPS(CCObject*) { FPSSettingsUI::open(); }

}  // namespace fbfc
