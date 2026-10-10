#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FPSSettingsUI : public geode::Popup<> {
public:
    static FPSSettingsUI* create();
    bool setup() override;

private:
    void onMacroFPSChanged(CCObject*);
    void onScanFPSChanged(CCObject*);
    void onPresetClicked(CCObject*);

    CCLabelBMFont* m_macroFPSLabel = nullptr;
    CCLabelBMFont* m_scanFPSLabel = nullptr;
    CCLabelBMFont* m_presetLabel = nullptr;
    Slider* m_macroSlider = nullptr;
    Slider* m_scanSlider = nullptr;
};