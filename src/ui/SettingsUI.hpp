#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

namespace fbfc {

// Menú modular con pestañas internas: Counter, Circles, Sounds, Scan, Recording, Tools.
class SettingsUI : public cocos2d::CCNode {
public:
    static SettingsUI* create(cocos2d::CCSize size);
private:
    bool init(cocos2d::CCSize size);
    void selectTab(int i);
    void onTab(cocos2d::CCObject*);
    void onToggle(cocos2d::CCObject*);
    void onCycle(cocos2d::CCObject*);
    void onFPS(cocos2d::CCObject*);

    cocos2d::CCSize m_size;
    cocos2d::CCNode* m_page = nullptr;
    cocos2d::CCMenu* m_tabMenu = nullptr;
    int m_tab = 0;
};

}  // namespace fbfc
