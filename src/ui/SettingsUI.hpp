#pragma once
#include <Geode/Geode.hpp>
#include "OlkFPC.hpp"

using namespace geode::prelude;

class SettingsUI : public geode::Popup {
public:
    static SettingsUI* create();
    bool setup();                    // ← sin override

private:
    int m_currentTab = 0;

    void createCounterTab();
    void createCirclesTab();
    void createSoundsTab();
    void createScanTab();
    void createRecordingTab();
    void createToolsTab();

    void addTitle(const char* text, float y);
    void addToggle(const char* label, const char* settingKey, float y);
    void addSlider(const char* label, const char* settingKey,
                   float min, float max, float y);
    void addDropdown(const char* label, const char* settingKey,
                     const std::vector<std::string>& options, float y);
    void addTextInput(const char* label, const char* settingKey, float y);
    void addDescription(const char* text, float y);

    void onTabClicked(CCObject* sender);
    void clearContent();

    CCLayerColor* m_contentBg = nullptr;
    std::vector<CCNode*> m_contentNodes;
};
