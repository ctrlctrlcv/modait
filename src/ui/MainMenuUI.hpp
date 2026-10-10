#pragma once
#include <Geode/Geode.hpp>
#include "../OlkFPC.hpp"
#include "../FrameAnalyzer.hpp"

using namespace geode::prelude;

class MainMenuUI : public geode::Popup<> {
public:
    static MainMenuUI* create();
    bool setup() override;

    // Secciones
    void showCounterTab();
    void showInsightsTab();
    void showSettingsTab();
    void showMacroGraph();

    // Botones
    void onImport(CCObject*);
    void onExport(CCObject*);
    void onFPS(CCObject*);
    void onSettings(CCObject*);
    void onPlayback(CCObject*);
    void onInsights(CCObject*);
    void onReload(CCObject*);

private:
    int m_currentTab = 0;

    CCLayerColor* m_contentBg = nullptr;
    std::vector<CCNode*> m_contentNodes;

    void clearContent();
    void rebuildContent();
    void refreshGraph();
};