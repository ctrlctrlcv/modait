#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

namespace fbfc {

class FPSSettingsUI : public geode::Popup {
public:
    static FPSSettingsUI* create();
    static void open();
protected:
    bool init();
private:
    void onPreset(cocos2d::CCObject*);
    void rebuild();
    cocos2d::CCNode* m_sliders = nullptr;
};

}  // namespace fbfc
