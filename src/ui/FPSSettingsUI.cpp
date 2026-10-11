#include "FPSSettingsUI.hpp"
#include "SettingWidgets.hpp"

using namespace geode::prelude;

namespace fbfc {

FPSSettingsUI* FPSSettingsUI::create() {
    auto* r = new FPSSettingsUI();
    if (r->init()) { r->autorelease(); return r; }
    delete r; return nullptr;
}
void FPSSettingsUI::open() { if (auto* p = create()) p->show(); }

bool FPSSettingsUI::init() {
    if (!Popup::init(300.f, 220.f)) return false;
    this->setTitle("FPS Settings");
    auto sz = m_mainLayer->getContentSize();
    rebuild();

    // Presets: aplican a macro-fps y scan-fps
    static constexpr int presets[] = {60, 120, 240, 480, 1000, 5000, 30000};
    auto* menu = CCMenu::create();
    menu->setPosition({sz.width / 2.f, 34.f});
    for (int p : presets) {
        auto* spr = ButtonSprite::create(std::to_string(p).c_str(), 38, true, "bigFont.fnt", "GJ_button_04.png", 18.f, 0.4f);
        auto* b = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FPSSettingsUI::onPreset));
        b->setTag(p);
        menu->addChild(b);
    }
    menu->setLayout(RowLayout::create()->setGap(2.f));
    m_mainLayer->addChild(menu);
    return true;
}

void FPSSettingsUI::rebuild() {
    if (m_sliders) m_sliders->removeFromParent();
    m_sliders = CCNode::create();
    auto sz = m_mainLayer->getContentSize();
    auto* a = SettingSlider::create("macro-fps", "Macro FPS", 1, 30000, true, true);
    a->setPosition({sz.width / 2.f - 95.f, sz.height - 85.f});
    auto* b = SettingSlider::create("scan-fps", "Scan FPS", 1, 30000, true, true);
    b->setPosition({sz.width / 2.f - 95.f, sz.height - 135.f});
    m_sliders->addChild(a); m_sliders->addChild(b);
    m_mainLayer->addChild(m_sliders);
}

void FPSSettingsUI::onPreset(CCObject* s) {
    int v = static_cast<CCNode*>(s)->getTag();
    (void)Mod::get()->setSettingValue<int64_t>("macro-fps", v);
    (void)Mod::get()->setSettingValue<int64_t>("scan-fps", v);
    rebuild();
}

}  // namespace fbfc
