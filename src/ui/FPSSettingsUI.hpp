#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FPSSettingsUI : public geode::Popup {
public:
    static FPSSettingsUI* create();
    bool setup();                          // ← sin override

private:
    void onPresetClicked(CCObject*);

    CCLabelBMFont* m_macroFPSLabel = nullptr;
    CCLabelBMFont* m_scanFPSLabel = nullptr;
    SliderNode* m_macroSlider = nullptr;
    SliderNode* m_scanSlider = nullptr;
};
