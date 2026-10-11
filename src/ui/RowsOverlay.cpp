#include "RowsOverlay.hpp"
#include "../OlkFPC.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace fbfc {

RowsOverlay* RowsOverlay::attach(PlayLayer* pl) {
    auto* r = new RowsOverlay();
    if (!r || !r->CCNode::init()) { delete r; return nullptr; }
    r->autorelease();
    auto win = CCDirector::get()->getWinSize();
    r->setPosition({6.f, win.height - 14.f});
    pl->addChild(r, 1000);
    return r;
}

std::vector<std::pair<int, int>> RowsOverlay::parseRanges(std::string const& s) {
    std::vector<std::pair<int, int>> out;
    size_t i = 0;
    while (i < s.size()) {
        size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        std::string tok = s.substr(i, j - i); i = j + 1;
        if (tok.empty()) continue;
        int a = 0, b = 0;
        if (tok.back() == '+') { a = std::atoi(tok.c_str()); b = 1 << 30; }
        else if (auto d = tok.find('-'); d != std::string::npos) {
            a = std::atoi(tok.c_str()); std::string rest = tok.substr(d + 1);
            b = (!rest.empty() && rest.back() == '+') ? (1 << 30) : std::atoi(rest.c_str());
        } else a = b = std::atoi(tok.c_str());
        out.emplace_back(a, b);
    }
    return out;
}

void RowsOverlay::refresh() {
    auto* mod = Mod::get();
    bool on = mod->getSettingValue<bool>("counter-enabled") && mod->getSettingValue<bool>("rows-enabled");
    this->setVisible(on);
    if (!on) return;
    auto& fpc = OlkFPC::get();
    auto const& live = fpc.liveRun().events;
    size_t presses = std::ranges::count_if(live, [](auto& e) { return e.down; });
    if (presses == m_lastCount && m_rev == fpc.revision()) return;
    m_lastCount = presses; m_rev = fpc.revision();
    this->removeAllChildren();

    auto mode = mod->getSettingValue<std::string>("rows-mode");
    size_t rows = mode == "Short" ? 4 : 8;
    auto ranges = mode == "Custom" ? parseRanges(mod->getSettingValue<std::string>("rows-custom")) : std::vector<std::pair<int, int>>{};
    float scale = static_cast<float>(mod->getSettingValue<double>("rows-size"));

    // Mapea cada push en vivo con el resultado del análisis (mismo índice de click).
    auto const& res = fpc.results();
    std::vector<std::pair<size_t, int>> shown;   // (n° de click, window o -2 si desconocido)
    size_t clickNo = 0;
    for (auto& e : live) {
        if (!e.down) continue;
        int w = clickNo < res.size() ? res[clickNo].window : -2;
        ++clickNo;
        if (mode == "Custom" && w >= 0 &&
            !std::ranges::any_of(ranges, [&](auto& r) { return w >= r.first && w <= r.second; })) continue;
        shown.emplace_back(clickNo, w);
    }
    size_t start = shown.size() > rows ? shown.size() - rows : 0;
    float y = 0.f;
    for (size_t i = start; i < shown.size(); ++i, y -= 18.f * scale * 2.f) {
        auto [n, w] = shown[i];
        auto* l = CCLabelBMFont::create(fmt::format("#{}  {}", n, w == -2 ? "--" : w == -1 ? "X" : std::to_string(w)).c_str(), "bigFont.fnt");
        l->setAnchorPoint({0.f, 0.5f}); l->setScale(scale); l->setPosition({0.f, y});
        l->setColor(w == -2 ? ccColor3B{160, 160, 160} : colorForWindow(w));
        this->addChild(l);
    }
}

}  // namespace fbfc
