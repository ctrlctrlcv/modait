#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FPSSettingsUI : public geode::Popup {   // ← Sin <>
public:
    static FPSSettingsUI* create();
    bool setup() override;

private:
    void onMacroFPSChanged(CCObject*);
    void onScanFPSChanged(CCObject*);
    void onPresetClicked(CCObject*);

    CCLabelBMFont* m_macroFPSLabel = nullptr;
    CCLabelBMFont* m_scanFPSLabel = nullptr;
    SliderNode* m_macroSlider = nullptr;   // ← SliderNode en vez de Slider
    SliderNode* m_scanSlider = nullptr;
};
