#include "MainMenuUI.hpp"
#include "FPSSettingsUI.hpp"
#include "SettingsUI.hpp"
#include "MacroParser.hpp"
#include <Geode/utils/file.hpp>

MainMenuUI* MainMenuUI::create() {
    auto ret = new MainMenuUI();
    if (ret && ret->init(720.0f, 520.0f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool MainMenuUI::setup() {
    auto bg = CCLayerColor::create({18, 18, 24, 255});
    bg->setContentSize(m_mainLayer->getContentSize());
    m_mainLayer->addChild(bg);

    auto titleBar = CCLayerColor::create({28, 28, 38, 255}, 720, 45);
    titleBar->setPosition(0, 475);
    m_mainLayer->addChild(titleBar);

    auto title = CCLabelBMFont::create("OlkFPC", "bigFont.fnt");
    title->setPosition(360, 497);
    title->setScale(0.75f);
    title->setColor({200, 200, 255});
    m_mainLayer->addChild(title);

    auto& olk = *OlkFPC::get();
    std::string macroInfo = olk.m_macroName.empty()
        ? "No macro cargado"
        : olk.m_macroName + " / " + olk.m_levelName;

    auto sub = CCLabelBMFont::create(macroInfo.c_str(), "chatFont.fnt");
    sub->setScale(0.4f);
    sub->setPosition(360, 470);
    sub->setColor({120, 120, 150});
    m_mainLayer->addChild(sub);

    const char* tabNames[] = {"Counter", "Insights", "Settings"};
    for (int i = 0; i < 3; ++i) {
        auto tabLabel = CCLabelBMFont::create(tabNames[i], "bigFont.fnt");
        tabLabel->setScale(0.5f);
        tabLabel->setColor(i == 0 ? ccColor3B{255, 255, 255} : ccColor3B{120, 120, 140});

        auto btn = CCMenuItemSpriteExtra::create(
            tabLabel, this, menu_selector(MainMenuUI::onTabClicked));
        btn->setTag(i);
        btn->setPosition(120 + i * 150, 445);

        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition(0, 0);
        m_mainLayer->addChild(menu);
    }

    auto importBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Import", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onImport));
    importBtn->setPosition(80, 40);

    auto exportBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Export", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onExport));
    exportBtn->setPosition(200, 40);

    auto fpsBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("FPS", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onFPS));
    fpsBtn->setPosition(320, 40);

    auto pbBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Playback", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onPlayback));
    pbBtn->setPosition(440, 40);

    auto reloadBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Reload", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onReload));
    reloadBtn->setPosition(560, 40);

    auto actionMenu = CCMenu::create();
    actionMenu->addChild(importBtn);
    actionMenu->addChild(exportBtn);
    actionMenu->addChild(fpsBtn);
    actionMenu->addChild(pbBtn);
    actionMenu->addChild(reloadBtn);
    actionMenu->setPosition(0, 0);
    m_mainLayer->addChild(actionMenu);

    m_contentBg = CCLayerColor::create({22, 22, 30, 255}, 660, 380);
    m_contentBg->setPosition(30, 60);
    m_mainLayer->addChild(m_contentBg);

    showCounterTab();
    return true;
}

void MainMenuUI::onTabClicked(CCObject* sender) {
    auto btn = dynamic_cast<CCMenuItemSpriteExtra*>(sender);
    if (!btn) return;

    m_currentTab = btn->getTag();
    clearContent();

    switch (m_currentTab) {
        case 0: showCounterTab(); break;
        case 1: showInsightsTab(); break;
        case 2: showSettingsTab(); break;
    }
}

void MainMenuUI::clearContent() {
    for (auto n : m_contentNodes) {
        if (n) n->removeFromParent();
    }
    m_contentNodes.clear();
}

void MainMenuUI::rebuildContent() {
    clearContent();
    switch (m_currentTab) {
        case 0: showCounterTab(); break;
        case 1: showInsightsTab(); break;
        case 2: showSettingsTab(); break;
    }
}

void MainMenuUI::showCounterTab() {
    m_currentTab = 0;
    clearContent();
    refreshGraph();
}

void MainMenuUI::refreshGraph() {
    auto& olk = *OlkFPC::get();
    auto& dist = olk.m_insights.frameWindowDistribution;

    if (dist.empty()) {
        auto noData = CCLabelBMFont::create(
            "No macro cargado.\nUsa 'Import' para cargar un .gdr, .cml, .ttr3 o .json",
            "chatFont.fnt");
        noData->setScale(0.5f);
        noData->setPosition(360, 250);
        noData->setColor({120, 120, 150});
        noData->setAlignment(kCCTextAlignmentCenter);
        m_contentNodes.push_back(noData);
        m_mainLayer->addChild(noData);
        return;
    }

    auto graphTitle = CCLabelBMFont::create(
        "FRAME WINDOW DISTRIBUTION", "bigFont.fnt");
    graphTitle->setScale(0.5f);
    graphTitle->setPosition(360, 410);
    graphTitle->setColor({180, 180, 220});
    m_contentNodes.push_back(graphTitle);
    m_mainLayer->addChild(graphTitle);

    float barWidth = 40.0f;
    float maxHeight = 180.0f;
    float startX = 100.0f;
    float baseY = 110.0f;
    float gap = 10.0f;

    int maxCount = 1;
    for (auto& d : dist) {
        if (d.count > maxCount) maxCount = d.count;
    }

    auto getColor = [](int window) -> cocos2d::ccColor3B {
        if (window == 0) return {255, 100, 100};
        if (window == 1) return {255, 165, 0};
        if (window == 2) return {255, 255, 100};
        if (window == 3) return {255, 255, 255};
        if (window == 4) return {150, 255, 150};
        if (window <= 6) return {100, 255, 255};
        if (window <= 8) return {100, 200, 255};
        return {200, 150, 255};
    };

    for (size_t i = 0; i < dist.size(); ++i) {
        auto& d = dist[i];
        float height = (static_cast<float>(d.count) / maxCount) * maxHeight;
        if (d.count == 0) height = 3.0f;

        auto color = getColor(d.window);
        auto bar = CCLayerColor::create(
            {color.r, color.g, color.b, 220}, barWidth, height);
        bar->setPosition(startX + i * (barWidth + gap), baseY);
        bar->setAnchorPoint({0.5f, 0.0f});
        m_contentNodes.push_back(bar);
        m_mainLayer->addChild(bar);

        std::string labelText = (d.window >= 9) ? "9+" : std::to_string(d.window);
        auto lbl = CCLabelBMFont::create(labelText.c_str(), "chatFont.fnt");
        lbl->setScale(0.5f);
        lbl->setPosition(startX + i * (barWidth + gap), baseY - 20);
        lbl->setColor(color);
        m_contentNodes.push_back(lbl);
        m_mainLayer->addChild(lbl);

        auto cnt = CCLabelBMFont::create(
            std::to_string(d.count).c_str(), "chatFont.fnt");
        cnt->setScale(0.45f);
        cnt->setPosition(startX + i * (barWidth + gap), baseY + height + 12);
        cnt->setColor({200, 200, 220});
        m_contentNodes.push_back(cnt);
        m_mainLayer->addChild(cnt);
    }

    auto& ins = olk.m_insights;
    std::string summary = fmt::format(
        "Total: {} | Exact: {} | Tightest: {} | Median: {}",
        ins.totalClicks, ins.exact, ins.tightest, ins.median);

    auto summaryLbl = CCLabelBMFont::create(summary.c_str(), "chatFont.fnt");
    summaryLbl->setScale(0.45f);
    summaryLbl->setPosition(360, 75);
    summaryLbl->setColor({180, 180, 220});
    m_contentNodes.push_back(summaryLbl);
    m_mainLayer->addChild(summaryLbl);
}

void MainMenuUI::showInsightsTab() {
    m_currentTab = 1;
    clearContent();

    auto& ins = OlkFPC::get()->m_insights;
    float statY = 380;

    struct StatInfo {
        const char* label;
        std::string value;
        cocos2d::ccColor3B color;
        const char* sub;
    };

    std::vector<StatInfo> stats = {
        {"EXACT", std::to_string(ins.exact), {100, 255, 200}, "of clicks"},
        {"TIGHTEST", ins.tightest >= 0 ? std::to_string(ins.tightest) : "-",
         {255, 150, 100}, "frames"},
        {"MEDIAN", std::to_string(ins.median), {150, 255, 100}, "frames"},
        {"BUSIEST", std::to_string(ins.busiestSecond), {255, 200, 100}, "clicks/sec"}
    };

    for (size_t i = 0; i < stats.size(); ++i) {
        auto& s = stats[i];
        float x = 120 + i * 140;

        auto lbl = CCLabelBMFont::create(s.label, "bigFont.fnt");
        lbl->setScale(0.35f);
        lbl->setPosition(x, statY);
        lbl->setColor({150, 150, 180});
        m_contentNodes.push_back(lbl);
        m_mainLayer->addChild(lbl);

        auto val = CCLabelBMFont::create(s.value.c_str(), "bigFont.fnt");
        val->setScale(0.65f);
        val->setPosition(x, statY - 25);
        val->setColor(s.color);
        m_contentNodes.push_back(val);
        m_mainLayer->addChild(val);

        auto sub = CCLabelBMFont::create(s.sub, "chatFont.fnt");
        sub->setScale(0.35f);
        sub->setPosition(x, statY - 45);
        sub->setColor({100, 100, 120});
        m_contentNodes.push_back(sub);
        m_mainLayer->addChild(sub);
    }

    auto hardTitle = CCLabelBMFont::create("HARDEST CLICKS", "bigFont.fnt");
    hardTitle->setScale(0.45f);
    hardTitle->setPosition(360, 300);
    hardTitle->setColor({180, 180, 220});
    m_contentNodes.push_back(hardTitle);
    m_mainLayer->addChild(hardTitle);

    float hy = 270;
    int count = 0;
    for (auto& hc : ins.hardestClicks) {
        if (count >= 10) break;

        cocos2d::ccColor3B color;
        if (hc.frameWindow == 0) color = {255, 100, 100};
        else if (hc.frameWindow == 1) color = {255, 165, 0};
        else if (hc.frameWindow == 2) color = {255, 255, 100};
        else if (hc.frameWindow == 3) color = {255, 255, 255};
        else if (hc.frameWindow <= 4) color = {150, 255, 150};
        else if (hc.frameWindow <= 6) color = {100, 255, 255};
        else if (hc.frameWindow <= 8) color = {100, 200, 255};
        else color = {200, 150, 255};

        auto line = CCLabelBMFont::create(
            fmt::format("FW {} | click #{} | {:.0f}% | {}",
                hc.frameWindow, hc.clickIndex, hc.percentage, hc.label).c_str(),
            "chatFont.fnt");
        line->setScale(0.45f);
        line->setPosition(360, hy);
        line->setColor(color);
        m_contentNodes.push_back(line);
        m_mainLayer->addChild(line);

        hy -= 22;
        count++;
    }
}

void MainMenuUI::showSettingsTab() {
    m_currentTab = 2;
    clearContent();

    auto lbl = CCLabelBMFont::create(
        "Configuración detallada disponible en 'Settings'",
        "chatFont.fnt");
    lbl->setScale(0.5f);
    lbl->setPosition(360, 250);
    lbl->setColor({150, 150, 180});
    m_contentNodes.push_back(lbl);
    m_mainLayer->addChild(lbl);

    auto hint = CCLabelBMFont::create(
        "Usa el botón de abajo para abrir todas las pestañas:\n"
        "Counter, Circles, Sounds, Scan, Recording, Tools",
        "chatFont.fnt");
    hint->setScale(0.4f);
    hint->setPosition(360, 200);
    hint->setColor({120, 120, 150});
    hint->setAlignment(kCCTextAlignmentCenter);
    m_contentNodes.push_back(hint);
    m_mainLayer->addChild(hint);

    auto settingsBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create("Open Full Settings", "bigFont.fnt"),
        this, menu_selector(MainMenuUI::onSettings));
    settingsBtn->setPosition(360, 130);
    settingsBtn->setScale(0.8f);

    auto menu = CCMenu::create();
    menu->addChild(settingsBtn);
    menu->setPosition(0, 0);
    m_contentNodes.push_back(menu);
    m_mainLayer->addChild(menu);
}

// ============================================================
//  IMPORT / EXPORT (versión simplificada)
// ============================================================

void MainMenuUI::onImport(CCObject*) {
    // Busca macros en la carpeta de guardado del mod
    auto dir = Mod::get()->getSaveDir();
    std::vector<std::filesystem::path> found;

    if (std::filesystem::exists(dir)) {
        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            auto ext = entry.path().extension().string();
            if (ext == ".gdr" || ext == ".gdr2" || ext == ".cml" ||
                ext == ".ttr3" || ext == ".json") {
                found.push_back(entry.path());
            }
        }
    }

    if (found.empty()) {
        Notification::create(
            "Coloca un macro (.gdr/.cml/.ttr3/.json) en:\n" + dir.string(),
            NotificationIcon::Info)->show();
        return;
    }

    // Cargar el primer macro encontrado
    if (OlkFPC::get()->loadMacro(found[0])) {
        rebuildContent();
    }
}

void MainMenuUI::onExport(CCObject*) {
    auto& olk = *OlkFPC::get();
    if (olk.m_clicks.empty()) {
        Notification::create("No hay macro para exportar", NotificationIcon::Warning)->show();
        return;
    }

    auto path = Mod::get()->getSaveDir() / "exported.gdr";
    auto res = MacroParser::exportTo(olk.m_clicks, path, MacroFormat::GDR);

    if (res.isErr()) {
        Notification::create(res.unwrapErr(), NotificationIcon::Error)->show();
    } else {
        Notification::create("Exportado a:\n" + path.string(), NotificationIcon::Success)->show();
    }
}

void MainMenuUI::onFPS(CCObject*) {
    FPSSettingsUI::create()->show();
}

void MainMenuUI::onSettings(CCObject*) {
    SettingsUI::create()->show();
}

void MainMenuUI::onPlayback(CCObject*) {
    auto& olk = *OlkFPC::get();
    if (olk.m_isPlaying) {
        olk.stopPlayback();
        Notification::create("Playback detenido", NotificationIcon::Info)->show();
    } else {
        olk.startPlayback();
        Notification::create("Playback iniciado", NotificationIcon::Info)->show();
    }
}

void MainMenuUI::onInsights(CCObject*) {
    showInsightsTab();
}

void MainMenuUI::onReload(CCObject*) {
    auto& olk = *OlkFPC::get();
    if (olk.m_macroPath.empty()) {
        Notification::create("No hay macro para recargar", NotificationIcon::Warning)->show();
        return;
    }
    olk.loadMacro(olk.m_macroPath);
    rebuildContent();
    Notification::create("Macro recargado", NotificationIcon::Success)->show();
}
