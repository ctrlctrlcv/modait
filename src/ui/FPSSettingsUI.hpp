#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FPSSettingsUI : public geode::Popup {
public:
    static FPSSettingsUI* create();
    bool init(float width, float height);      // ← init, no setup

private:
    void buildUI();                            // ← antes era setup
    void onPresetClicked(CCObject*);

    CCLabelBMFont* m_macroFPSLabel = nullptr;
    CCLabelBMFont* m_scanFPSLabel = nullptr;
    SliderNode* m_macroSlider = nullptr;
    SliderNode* m_scanSlider = nullptr;
};
