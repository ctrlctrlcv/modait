#pragma once
#include <Geode/Geode.hpp>
#include "OlkFPC.hpp"

using namespace geode::prelude;

class MainMenuUI : public geode::Popup {
public:
    static MainMenuUI* create();
    bool init(float width, float height);      // ← sobreescribir init, NO setup

    void showCounterTab();
    void showInsightsTab();
    void showSettingsTab();
    void showMacroGraph();

    void onImport(CCObject*);
    void onExport(CCObject*);
    void onFPS(CCObject*);
    void onSettings(CCObject*);
    void onPlayback(CCObject*);
    void onInsights(CCObject*);
    void onReload(CCObject*);
    void onTabClicked(CCObject*);

private:
    int m_currentTab = 0;

    CCLayerColor* m_contentBg = nullptr;
    std::vector<CCNode*> m_contentNodes;

    void buildUI();                            // ← contenido (antes setup)
    void clearContent();
    void rebuildContent();
    void refreshGraph();
};
