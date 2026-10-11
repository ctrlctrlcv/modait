#pragma once
#include <Geode/Geode.hpp>

namespace fbfc {

// Sliders logarítmicos (1..30000) + presets para macro-fps y scan-fps.
class FPSSettingsUI : public geode::Popup<> {
public:
    static FPSSettingsUI* create();
    static void open();
protected:
    bool setup() override;
private:
    void onPreset(cocos2d::CCObject*);
    void rebuild();
    cocos2d::CCNode* m_sliders = nullptr;
};

}  // namespace fbfc
