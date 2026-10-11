#include "FPSSettingsUI.hpp"
#include <Geode/utils/string.hpp>
#include <cmath>
#include <algorithm>

FPSSettingsUI* FPSSettingsUI::create() {
    auto ret = new FPSSettingsUI();
    if (ret && ret->init(500.0f, 400.0f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool FPSSettingsUI::setup() {
    auto bg = CCLayerColor::create({18, 18, 24, 255});
    bg->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(bg);

    auto mod = Mod::get();

    auto title = CCLabelBMFont::create("FPS Configuration", "bigFont.fnt");
    title->setScale(0.7f);
    title->setPosition(250, 360);
    title->setColor({200, 200, 255});
    m_mainLayer->addChild(title);

    auto subtitle = CCLabelBMFont::create("Customizable: 1 - 30000 FPS", "chatFont.fnt");
    subtitle->setScale(0.4f);
    subtitle->setPosition(250, 335);
    subtitle->setColor({120, 120, 150});
    m_mainLayer->addChild(subtitle);

    // ============ MACRO FPS ============
    int macroFps = static_cast<int>(mod->getSettingValue<int64_t>("macro-fps"));

    auto macroLbl = CCLabelBMFont::create("Macro FPS", "chatFont.fnt");
    macroLbl->setScale(0.5f);
    macroLbl->setPosition(60, 270);
    macroLbl->setAnchorPoint({0.0f, 0.5f});
    m_mainLayer->addChild(macroLbl);

    m_macroFPSLabel = CCLabelBMFont::create(std::to_string(macroFps).c_str(), "bigFont.fnt");
    m_macroFPSLabel->setScale(0.6f);
    m_macroFPSLabel->setPosition(430, 270);
    m_macroFPSLabel->setColor({100, 255, 200});
    m_mainLayer->addChild(m_macroFPSLabel);

    // ✅ Callback con DOS parámetros (SliderNode*, float)
    m_macroSlider = SliderNode::create([this](SliderNode* node, float value) {
        int fps = static_cast<int>(std::pow(10.0f, value * std::log10(30000.0f)));
        fps = std::clamp(fps, 1, 30000);
        m_macroFPSLabel->setString(std::to_string(fps).c_str());
        Mod::get()->setSettingValue<int64_t>("macro-fps", fps);
    });
    m_macroSlider->setPosition(250, 230);
    m_macroSlider->setMin(0.0f);
    m_macroSlider->setMax(1.0f);
    m_macroSlider->setValue(std::log10(static_cast<float>(macroFps)) / std::log10(30000.0f));
    m_mainLayer->addChild(m_macroSlider);

    // Presets
    auto presetMenu = CCMenu::create();
    presetMenu->setPosition(0, 0);
    m_mainLayer->addChild(presetMenu);

    const std::vector<int> presets = {60, 120, 240, 480, 960, 1000};
    for (size_t i = 0; i < presets.size(); ++i) {
        auto label = CCLabelBMFont::create(std::to_string(presets[i]).c_str(), "chatFont.fnt");
        label->setScale(0.45f);
        auto btn = CCMenuItemSpriteExtra::create(label, this,
            menu_selector(FPSSettingsUI::onPresetClicked));
        btn->setTag(presets[i]);
        btn->setPosition(80 + i * 70, 190);
        presetMenu->addChild(btn);
    }

    // ============ SCAN FPS ============
    int scanFps = static_cast<int>(mod->getSettingValue<int64_t>("scan-fps"));

    auto scanLbl = CCLabelBMFont::create("Scan FPS", "chatFont.fnt");
    scanLbl->setScale(0.5f);
    scanLbl->setPosition(60, 140);
    scanLbl->setAnchorPoint({0.0f, 0.5f});
    m_mainLayer->addChild(scanLbl);

    m_scanFPSLabel = CCLabelBMFont::create(std::to_string(scanFps).c_str(), "bigFont.fnt");
    m_scanFPSLabel->setScale(0.6f);
    m_scanFPSLabel->setPosition(430, 140);
    m_scanFPSLabel->setColor({255, 200, 100});
    m_mainLayer->addChild(m_scanFPSLabel);

    m_scanSlider = SliderNode::create([this](SliderNode* node, float value) {
        int fps = static_cast<int>(std::pow(10.0f, value * std::log10(30000.0f)));
        fps = std::clamp(fps, 1, 30000);
        m_scanFPSLabel->setString(std::to_string(fps).c_str());
        Mod::get()->setSettingValue<int64_t>("scan-fps", fps);
    });
    m_scanSlider->setPosition(250, 100);
    m_scanSlider->setMin(0.0f);
    m_scanSlider->setMax(1.0f);
    m_scanSlider->setValue(std::log10(static_cast<float>(scanFps)) / std::log10(30000.0f));
    m_mainLayer->addChild(m_scanSlider);

    auto warning = CCLabelBMFont::create(
        "Valores altos aumentan el tiempo de análisis", "chatFont.fnt");
    warning->setScale(0.35f);
    warning->setPosition(250, 50);
    warning->setColor({255, 150, 100});
    m_mainLayer->addChild(warning);

    auto resetBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Reset to 240", "chatFont.fnt"),
        this, menu_selector(FPSSettingsUI::onPresetClicked));
    resetBtn->setTag(240);
    auto resetMenu = CCMenu::create();
    resetMenu->addChild(resetBtn);
    resetMenu->setPosition(250, 25);
    m_mainLayer->addChild(resetMenu);

    return true;
}

void FPSSettingsUI::onPresetClicked(CCObject* sender) {
    auto btn = dynamic_cast<CCMenuItemSpriteExtra*>(sender);
    if (!btn) return;

    int fps = btn->getTag();
    if (fps <= 0) fps = 240;

    m_macroFPSLabel->setString(std::to_string(fps).c_str());
    m_scanFPSLabel->setString(std::to_string(fps).c_str());
    Mod::get()->setSettingValue<int64_t>("macro-fps", fps);
    Mod::get()->setSettingValue<int64_t>("scan-fps", fps);

    m_macroSlider->setValue(std::log10(static_cast<float>(fps)) / std::log10(30000.0f));
    m_scanSlider->setValue(std::log10(static_cast<float>(fps)) / std::log10(30000.0f));
}
