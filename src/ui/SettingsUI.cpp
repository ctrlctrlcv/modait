#include "SettingsUI.hpp"
#include <Geode/utils/string.hpp>
#include <algorithm>

// ============================================================
//  CREACIÓN
// ============================================================

SettingsUI* SettingsUI::create() {
    auto ret = new SettingsUI();
    if (ret && ret->init(640.0f, 480.0f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SettingsUI::init(float width, float height) {
    if (!Popup::init(width, height)) return false;
    buildUI();
    return true;
}

// ============================================================
//  BUILD UI
// ============================================================

void SettingsUI::buildUI() {
    auto bg = CCLayerColor::create({16, 16, 22, 255});
    bg->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(bg);

    auto title = CCLabelBMFont::create("OlkFPC Settings", "bigFont.fnt");
    title->setScale(0.6f);
    title->setPosition(320, 450);
    title->setColor({200, 200, 255});
    m_mainLayer->addChild(title);

    const char* tabs[] = {"Counter", "Circles", "Sounds", "Scan", "Recording", "Tools"};

    auto tabMenu = CCMenu::create();
    tabMenu->setPosition(0, 0);
    m_mainLayer->addChild(tabMenu);

    for (int i = 0; i < 6; ++i) {
        auto tabLabel = CCLabelBMFont::create(tabs[i], "bigFont.fnt");
        tabLabel->setScale(0.45f);
        tabLabel->setColor(i == 0 ? ccColor3B{255, 255, 255} : ccColor3B{100, 100, 120});

        auto btn = CCMenuItemSpriteExtra::create(
            tabLabel, this, menu_selector(SettingsUI::onTabClicked));
        btn->setTag(i);
        btn->setPosition(60, 400 - i * 55);
        tabMenu->addChild(btn);
    }

    m_contentBg = CCLayerColor::create({22, 22, 30, 255}, 500, 420);
    m_contentBg->setPosition(110, 20);
    m_mainLayer->addChild(m_contentBg);

    createCounterTab();
}

// ============================================================
//  CONTROL DE PESTAÑAS
// ============================================================

void SettingsUI::onTabClicked(CCObject* sender) {
    auto btn = dynamic_cast<CCMenuItemSpriteExtra*>(sender);
    if (!btn) return;

    m_currentTab = btn->getTag();
    clearContent();

    switch (m_currentTab) {
        case 0: createCounterTab(); break;
        case 1: createCirclesTab(); break;
        case 2: createSoundsTab(); break;
        case 3: createScanTab(); break;
        case 4: createRecordingTab(); break;
        case 5: createToolsTab(); break;
    }

    // Colores de las pestañas
    for (CCNode* child : m_mainLayer->getChildrenExt()) {
        auto menu = dynamic_cast<CCMenu*>(child);
        if (!menu) continue;

        for (CCNode* menuChild : menu->getChildrenExt()) {
            auto item = dynamic_cast<CCMenuItemSpriteExtra*>(menuChild);
            if (!item) continue;

            int tag = item->getTag();
            if (tag < 0 || tag > 5) continue;

            auto label = dynamic_cast<CCLabelBMFont*>(item->getNormalImage());
            if (!label) continue;

            if (tag == m_currentTab) {
                label->setColor({255, 255, 255});
            } else {
                label->setColor({100, 100, 120});
            }
        }
    }
}

void SettingsUI::clearContent() {
    for (auto n : m_contentNodes) {
        if (n) n->removeFromParent();
    }
    m_contentNodes.clear();
}

// ============================================================
//  HELPERS
// ============================================================

void SettingsUI::addTitle(const char* text, float y) {
    auto title = CCLabelBMFont::create(text, "bigFont.fnt");
    title->setScale(0.4f);
    title->setPosition(360, y);
    title->setColor({180, 180, 220});
    m_contentNodes.push_back(title);
    m_mainLayer->addChild(title);
}

void SettingsUI::addDescription(const char* text, float y) {
    auto desc = CCLabelBMFont::create(text, "chatFont.fnt");
    desc->setScale(0.3f);
    desc->setPosition(360, y);
    desc->setColor({120, 120, 150});
    m_contentNodes.push_back(desc);
    m_mainLayer->addChild(desc);
}

void SettingsUI::addToggle(const char* label, const char* settingKey, float y) {
    auto mod = Mod::get();
    bool value = mod->getSettingValue<bool>(settingKey);

    auto lbl = CCLabelBMFont::create(label, "chatFont.fnt");
    lbl->setScale(0.5f);
    lbl->setPosition(130, y);
    lbl->setAnchorPoint({0.0f, 0.5f});
    m_contentNodes.push_back(lbl);
    m_mainLayer->addChild(lbl);

    auto toggle = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this, nullptr);
    toggle->setPosition(550, y);
    toggle->setScale(0.7f);
    toggle->toggle(value);

    auto menu = CCMenu::create();
    menu->addChild(toggle);
    menu->setPosition(0, 0);
    m_contentNodes.push_back(menu);
    m_mainLayer->addChild(menu);
}

void SettingsUI::addSlider(const char* label, const char* settingKey,
                            float minVal, float maxVal, float y) {
    auto mod = Mod::get();
    double value = mod->getSettingValue<double>(settingKey);

    auto lbl = CCLabelBMFont::create(label, "chatFont.fnt");
    lbl->setScale(0.5f);
    lbl->setPosition(130, y + 10);
    lbl->setAnchorPoint({0.0f, 0.5f});
    m_contentNodes.push_back(lbl);
    m_mainLayer->addChild(lbl);

    auto valLabel = CCLabelBMFont::create(
        fmt::format("{:.1f}", value).c_str(), "chatFont.fnt");
    valLabel->setScale(0.4f);
    valLabel->setPosition(550, y - 10);
    valLabel->setColor({200, 200, 255});
    m_contentNodes.push_back(valLabel);
    m_mainLayer->addChild(valLabel);

    std::string keyCopy = settingKey;
    float range = maxVal - minVal;

    auto slider = SliderNode::create([keyCopy, valLabel, minVal, range]
                                       (SliderNode* node, float v) {
        double newVal = minVal + v * range;
        valLabel->setString(fmt::format("{:.1f}", newVal).c_str());
        Mod::get()->setSettingValue<double>(keyCopy, newVal);
    });

    slider->setPosition(360, y - 10);
    slider->setMin(0.0f);
    slider->setMax(1.0f);
    float normalized = (range > 0.0f)
        ? static_cast<float>((value - minVal) / range)
        : 0.0f;
    slider->setValue(std::clamp(normalized, 0.0f, 1.0f));

    m_contentNodes.push_back(slider);
    m_mainLayer->addChild(slider);
}

void SettingsUI::addDropdown(const char* label, const char* settingKey,
                              const std::vector<std::string>& options, float y) {
    auto mod = Mod::get();
    std::string current = mod->getSettingValue<std::string>(settingKey);

    auto lbl = CCLabelBMFont::create(label, "chatFont.fnt");
    lbl->setScale(0.5f);
    lbl->setPosition(130, y);
    lbl->setAnchorPoint({0.0f, 0.5f});
    m_contentNodes.push_back(lbl);
    m_mainLayer->addChild(lbl);

    auto currentLabel = CCLabelBMFont::create(current.c_str(), "chatFont.fnt");
    currentLabel->setScale(0.45f);
    currentLabel->setPosition(450, y);
    currentLabel->setColor({200, 200, 255});
    m_contentNodes.push_back(currentLabel);
    m_mainLayer->addChild(currentLabel);

    auto cycleBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create(">", "bigFont.fnt"),
        this, nullptr);
    cycleBtn->setPosition(540, y);
    cycleBtn->setScale(0.6f);

    auto menu = CCMenu::create();
    menu->addChild(cycleBtn);
    menu->setPosition(0, 0);
    m_contentNodes.push_back(menu);
    m_mainLayer->addChild(menu);
}

void SettingsUI::addTextInput(const char* label, const char* settingKey, float y) {
    auto mod = Mod::get();
    std::string value = mod->getSettingValue<std::string>(settingKey);

    auto lbl = CCLabelBMFont::create(label, "chatFont.fnt");
    lbl->setScale(0.5f);
    lbl->setPosition(130, y);
    lbl->setAnchorPoint({0.0f, 0.5f});
    m_contentNodes.push_back(lbl);
    m_mainLayer->addChild(lbl);

    auto input = TextInput::create(300, "value", "chatFont.fnt");
    input->setString(value);
    input->setPosition(360, y);
    m_contentNodes.push_back(input);
    m_mainLayer->addChild(input);
}

// ============================================================
//  PESTAÑAS
// ============================================================

void SettingsUI::createCounterTab() {
    float y = 400;
    addTitle("ROWS IN THE TOP CORNER", y);
    y -= 30;
    addDropdown("Rows Mode", "rows-mode", {"Short", "Normal", "Personalize"}, y);
    y -= 25;
    addDescription("Short: 1,2,3,4,5+ | Normal: 13-15,9-12,7-8,5-6,4,3,2,1", y);
    y -= 35;
    addToggle("Copy Counts", "copy-counts", y);
    y -= 35;
    addToggle("Count Straight Fly", "count-straight-fly", y);
    y -= 35;
    addToggle("Count Wave Spam", "count-wave-spam", y);
    y -= 35;
    addTitle("CUSTOM ROWS (Personalize)", y);
    y -= 30;
    addTextInput("Rows", "rows-custom", y);
}

void SettingsUI::createCirclesTab() {
    float y = 400;
    addTitle("CIRCLE APPEARANCE", y);
    y -= 40;
    addSlider("Circle Size (%)", "circle-size", 20.0f, 300.0f, y);
    y -= 60;
    addSlider("Number Size (%)", "number-size", 20.0f, 300.0f, y);
    y -= 60;
    addToggle("Milliseconds Under Number", "ms-under-number", y);
    y -= 40;
    addToggle("Number Beside Ring", "number-beside-ring", y);
}

void SettingsUI::createSoundsTab() {
    float y = 400;
    addTitle("SOUND CONFIGURATION", y);
    y -= 40;
    addToggle("Vary Pitch", "vary-pitch", y);
    y -= 40;
    addDropdown("Sound Timing", "sound-timing",
                {"On Click", "Before Click", "After Click", "Perfect Frame"}, y);
    y -= 40;
    addDescription("El pitch del BING! varía según la precisión del click", y);
}

void SettingsUI::createScanTab() {
    float y = 400;
    addTitle("SCAN CONFIGURATION", y);
    y -= 40;
    addDropdown("Frames Each Side", "scan-distance",
                {"Auto", "5", "10", "15", "20", "25", "30", "50"}, y);
    y -= 40;
    addSlider("From (%)", "scan-from", 0.0f, 100.0f, y);
    y -= 60;
    addSlider("To (%)", "scan-to", 0.0f, 100.0f, y);
    y -= 60;
    addDropdown("Scan Speed", "scan-speed", {"FULL", "FAST", "FASTEST"}, y);
    y -= 40;
    addToggle("Local Check", "local-check", y);
    y -= 30;
    addDescription("A click passes when alive after the next click", y);
}

void SettingsUI::createRecordingTab() {
    float y = 400;
    addTitle("RECORDING & PLAYBACK", y);
    y -= 40;
    addSlider("Recording Speed (%)", "recording-speed", 20.0f, 500.0f, y);
    y -= 60;
    addSlider("Playback Speed (%)", "playback-speed", 20.0f, 500.0f, y);
    y -= 60;
    addToggle("Click During Playback", "click-during-playback", y);
    y -= 30;
    addDescription("Desactivado por defecto", y);
}

void SettingsUI::createToolsTab() {
    float y = 400;
    addTitle("TOOLS", y);
    y -= 40;
    addToggle("Hitboxes", "show-hitboxes", y);
    y -= 32;
    addToggle("Hitboxes on Death", "hitboxes-on-death", y);
    y -= 32;
    addToggle("Hitbox Trail", "hitbox-trail", y);
    y -= 32;
    addToggle("Trajectory", "show-trajectory", y);
    y -= 32;
    addToggle("Frame Stepper", "frame-stepper", y);
    y -= 32;
    addToggle("Input Display", "input-display", y);
    y -= 40;
    addDropdown("Render Preset", "render-preset",
                {"720p30", "720p60", "1080p30", "1080p60",
                 "1440p60", "4K30", "4K60", "Custom"}, y);
}
