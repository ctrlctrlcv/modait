#include "MainMenuUI.hpp"
#include "SettingsUI.hpp"
#include "../MacroParser.hpp"
#include "../OlkFPC.hpp"
#include <Geode/utils/file.hpp>
#include <algorithm>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

namespace fbfc {

MainMenuUI* MainMenuUI::create() {
    auto* r = new MainMenuUI();
    if (r->init()) { r->autorelease(); return r; }
    delete r; return nullptr;
}
void MainMenuUI::open() { if (auto* p = create()) p->show(); }

bool MainMenuUI::init() {
    if (!Popup::init(460.f, 300.f)) return false;
    this->setTitle("FBFrameCounter");
    // ... el resto de la función queda igual
    auto sz = m_mainLayer->getContentSize();
    static constexpr char const* names[] = {"Counter", "Insights", "Settings"};
    for (int i = 0; i < 3; ++i) {
        auto* spr = ButtonSprite::create(names[i], 80, true, "bigFont.fnt", "GJ_button_04.png", 24.f, 0.5f);
        auto* b = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MainMenuUI::onTab));
        b->setTag(i);
        b->setPosition({sz.width / 2.f - 90.f + i * 90.f, sz.height - 44.f});
        m_buttonMenu->addChild(b);
    }
    // Acciones: analizar / importar / exportar
    static constexpr struct { char const* t; SEL_MenuHandler h; } acts[] = {
        {"Analyze", menu_selector(MainMenuUI::onAnalyze)},
        {"Import", menu_selector(MainMenuUI::onImport)},
        {"Export", menu_selector(MainMenuUI::onExport)}};
    for (int i = 0; i < 3; ++i) {
        auto* spr = ButtonSprite::create(acts[i].t, 70, true, "goldFont.fnt", "GJ_button_01.png", 24.f, 0.6f);
        auto* b = CCMenuItemSpriteExtra::create(spr, this, acts[i].h);
        b->setPosition({sz.width / 2.f - 80.f + i * 80.f, 22.f});
        m_buttonMenu->addChild(b);
    }
    m_status = CCLabelBMFont::create("", "bigFont.fnt");
    m_status->setScale(0.3f);
    m_status->setPosition({sz.width / 2.f, 46.f});
    m_mainLayer->addChild(m_status);

    select(Tab::Counter);
    this->scheduleUpdate();
    return true;
}

void MainMenuUI::onTab(CCObject* s) { select(static_cast<Tab>(static_cast<CCNode*>(s)->getTag())); }

void MainMenuUI::select(Tab t) {
    m_tab = t; m_rev = UINT64_MAX;
    if (m_content) m_content->removeFromParent();
    m_content = CCNode::create();
    m_mainLayer->addChild(m_content);
    auto sz = m_mainLayer->getContentSize();
    switch (t) {
        case Tab::Counter: buildCounter(); break;
        case Tab::Insights: buildInsights(); break;
        case Tab::Settings: {
            auto* s = SettingsUI::create({sz.width - 30.f, sz.height - 120.f});
            s->setPosition({15.f, 60.f});
            m_content->addChild(s);
        } break;
    }
}

void MainMenuUI::update(float) {
    auto& fpc = OlkFPC::get();
    if (fpc.isAnalyzing()) m_status->setString(fmt::format("Analyzing... {:.0f}%", fpc.analysisProgress() * 100.f).c_str());
    else if (m_rev != UINT64_MAX && m_status->getString()[0] == 'A') m_status->setString("");
    if (m_tab != Tab::Settings && m_rev != fpc.revision() && !fpc.isAnalyzing()) select(m_tab), m_rev = fpc.revision();
}

// ---- Counter: gráfico de barras de distribución + resumen
void MainMenuUI::buildCounter() {
    auto const& ins = OlkFPC::get().insights();
    auto sz = m_mainLayer->getContentSize();
    float chartW = 360.f, chartH = 120.f, x0 = (sz.width - chartW) / 2.f, y0 = 100.f;
    size_t mx = std::max<size_t>(1, *std::ranges::max_element(ins.buckets));
    static constexpr int rep[8] = {0, 1, 2, 3, 4, 5, 7, 9};
    float slot = chartW / 8.f;
    for (int i = 0; i < 8; ++i) {
        float h = std::max(2.f, chartH * static_cast<float>(ins.buckets[i]) / static_cast<float>(mx));
        auto* bar = CCLayerColor::create(ccColor4B{colorForWindow(rep[i]).r, colorForWindow(rep[i]).g, colorForWindow(rep[i]).b, 255}, slot - 8.f, h);
        bar->setPosition({x0 + i * slot + 4.f, y0});
        m_content->addChild(bar);
        auto* n = CCLabelBMFont::create(std::to_string(ins.buckets[i]).c_str(), "bigFont.fnt");
        n->setScale(0.35f); n->setPosition({x0 + i * slot + slot / 2.f, y0 + h + 8.f});
        m_content->addChild(n);
        auto* l = CCLabelBMFont::create(kBucketLabels[i], "bigFont.fnt");
        l->setScale(0.4f); l->setColor(colorForWindow(rep[i])); l->setPosition({x0 + i * slot + slot / 2.f, y0 - 10.f});
        m_content->addChild(l);
    }
    auto* sum = CCLabelBMFont::create(
        ins.total ? fmt::format("{} clicks  |  {} exact  |  median {:.1f}f", ins.total, ins.exact, ins.median).c_str()
                  : "No data. Play a run, then press Analyze.", "bigFont.fnt");
    sum->setScale(0.38f); sum->setPosition({sz.width / 2.f, y0 + chartH + 22.f});
    m_content->addChild(sum);
}

// ---- Insights: métricas globales + top 10 más difíciles
void MainMenuUI::buildInsights() {
    auto const& ins = OlkFPC::get().insights();
    auto sz = m_mainLayer->getContentSize();
    auto line = [&](std::string const& s, float x, float y, float sc, ccColor3B c = {255, 255, 255}) {
        auto* l = CCLabelBMFont::create(s.c_str(), "bigFont.fnt");
        l->setScale(sc); l->setColor(c); l->setAnchorPoint({0.f, 0.5f}); l->setPosition({x, y});
        m_content->addChild(l);
    };
    if (!ins.total) { line("No data yet.", 30.f, sz.height - 90.f, 0.45f); return; }
    float y = sz.height - 80.f;
    line(fmt::format("Total clicks: {}", ins.total), 24.f, y, 0.4f);
    line(fmt::format("0-frame clicks: {}", ins.exact), 24.f, y - 20.f, 0.4f);
    line(fmt::format("Tightest window: {}f", ins.tightest), 24.f, y - 40.f, 0.4f, colorForWindow(ins.tightest));
    line(fmt::format("Median: {:.1f}f", ins.median), 24.f, y - 60.f, 0.4f);
    line(fmt::format("Rhythm: {:.2f} clicks/s", ins.clicksPerSec), 24.f, y - 80.f, 0.4f);
    line("Top 10 hardest", 250.f, y, 0.4f, {255, 220, 100});
    for (size_t i = 0; i < ins.hardest.size(); ++i) {
        auto& r = ins.hardest[i];
        char const* tag = r.pattern == PatternTag::WaveSpam ? " [wave]" : r.pattern == PatternTag::StraightFly ? " [fly]" : "";
        line(fmt::format("{}. f{}  {}f  P{}{}", i + 1, r.frame, r.window, r.player2 ? 2 : 1, tag), 250.f, y - 18.f - i * 15.f, 0.3f, colorForWindow(r.window));
    }
}

void MainMenuUI::onAnalyze(CCObject*) {
    auto r = OlkFPC::get().requestAnalysis();
    if (!r) FLAlertLayer::create("Analyzer", r.unwrapErr(), "OK")->show();
}

void MainMenuUI::onImport(CCObject*) {
    file::FilePickOptions opts;
    opts.filters = {{"Macros", {"*.gdr", "*.json", "*.gdr2", "*.cml", "*.ttr3"}}};
    m_pick.bind([this](auto* ev) {
        auto* res = ev->getValue();
        if (!res || !res->isOk()) return;
        auto m = MacroParser::load(res->unwrap());
        if (!m) { FLAlertLayer::create("Import failed", m.unwrapErr(), "OK")->show(); return; }
        auto r = OlkFPC::get().requestAnalysis(std::move(m).unwrap());
        if (!r) FLAlertLayer::create("Analyzer", r.unwrapErr(), "OK")->show();
    });
    m_pick.setFilter(file::pick(file::PickMode::OpenFile, opts));
}

void MainMenuUI::onExport(CCObject*) {
    auto const& m = OlkFPC::get().lastRun().empty() ? OlkFPC::get().liveRun() : OlkFPC::get().lastRun();
    if (m.empty()) { FLAlertLayer::create("Export", "Nothing recorded yet.", "OK")->show(); return; }
    file::FilePickOptions opts;
    opts.defaultPath = "macro.json";
    opts.filters = {{"Macros", {"*.gdr", "*.json", "*.gdr2", "*.cml", "*.ttr3"}}};
    MacroData copy = m;
    m_pick.bind([copy](auto* ev) {
        auto* res = ev->getValue();
        if (!res || !res->isOk()) return;
        auto w = MacroParser::save(copy, res->unwrap());
        Notification::create(w ? "Macro exported" : w.unwrapErr(), w ? NotificationIcon::Success : NotificationIcon::Error)->show();
    });
    m_pick.setFilter(file::pick(file::PickMode::SaveFile, opts));
}

}  // namespace fbfc
