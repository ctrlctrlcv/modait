#pragma once
#include "../Types.hpp"

namespace fbfc {

class SettingsUI;

// Popup principal: pestañas Counter / Insights / Settings.
class MainMenuUI : public geode::Popup<> {
public:
    static MainMenuUI* create();
    static void open();
protected:
    bool setup() override;
    void update(float dt) override;
private:
    enum class Tab { Counter, Insights, Settings };
    void select(Tab t);
    void buildCounter();
    void buildInsights();
    void onTab(cocos2d::CCObject*);
    void onAnalyze(cocos2d::CCObject*);
    void onImport(cocos2d::CCObject*);
    void onExport(cocos2d::CCObject*);

    Tab m_tab = Tab::Counter;
    cocos2d::CCNode* m_content = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    uint64_t m_rev = UINT64_MAX;
    geode::EventListener<geode::Task<geode::Result<std::filesystem::path>>> m_pick;
};

}  // namespace fbfc
