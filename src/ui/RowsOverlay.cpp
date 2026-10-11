#include "RowsOverlay.hpp"
#include <Geode/utils/string.hpp>
#include <algorithm>
#include <map>

RowsOverlay* RowsOverlay::create() {
    auto ret = new RowsOverlay();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool RowsOverlay::init() {
    if (!CCNode::init()) return false;

    auto winSize = CCDirector::get()->getWinSize();
    m_baseX = 20.0f;
    m_baseY = winSize.height - 40.0f;

    this->setPosition(0, 0);
    this->scheduleUpdate();
    return true;
}

void RowsOverlay::update(float dt) {
    if (!m_visible) {
        this->setVisible(false);
        return;
    }
    this->setVisible(true);

    auto& olk = *OlkFPC::get();
    if (!olk.m_analysisDone) return;

    updateRowLabels();
}

void RowsOverlay::refresh() {
    m_dirty = true;
    updateRowLabels();
}

std::vector<int> RowsOverlay::getConfiguredRows() const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    if (mode == "Short") {
        return { 1, 2, 3, 4, 5 };
    }

    if (mode == "Normal") {
        return { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    std::string custom = mod->getSettingValue<std::string>("rows-custom");
    return parseCustomRows(custom);
}

std::vector<int> RowsOverlay::parseCustomRows(const std::string& custom) const {
    std::vector<int> rows;

    auto parts = geode::utils::string::split(custom, ",");
    for (auto& part : parts) {
        part = geode::utils::string::trim(part);
        if (part.empty()) continue;

        if (part.find('-') != std::string::npos) {
            auto range = geode::utils::string::split(part, "-");
            if (range.size() == 2) {
                try {
                    int start = std::stoi(geode::utils::string::trim(range[0]));
                    rows.push_back(start);
                } catch (...) {
                    log::warn("OlkFPC: Rango inválido: {}", part);
                }
            }
        } else {
            try {
                rows.push_back(std::stoi(part));
            } catch (...) {
                log::warn("OlkFPC: Valor inválido: {}", part);
            }
        }
    }

    if (rows.empty()) {
        rows = { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    return rows;
}

cocos2d::ccColor3B RowsOverlay::getColorForWindow(int window) const {
    if (window == 0)      return { 255, 100, 100 };
    if (window == 1)      return { 255, 165, 0 };
    if (window == 2)      return { 255, 255, 100 };
    if (window == 3)      return { 255, 255, 255 };
    if (window == 4)      return { 150, 255, 150 };
    if (window <= 6)      return { 100, 255, 255 };
    if (window <= 8)      return { 100, 200, 255 };
    return { 200, 150, 255 };
}

std::string RowsOverlay::getLabelForWindow(int window,
                                            const std::map<int, int>& counts) const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    int count = 0;
    std::string label;

    if (mode == "Short" && window == 5) {
        for (auto& [w, c] : counts) {
            if (w >= 5) count += c;
        }
        label = "5+";
    }
    else if (mode == "Normal") {
        if (window == 13) {
            for (auto& [w, c] : counts) {
                if (w >= 13 && w <= 15) count += c;
            }
            label = "13-15";
        }
        else if (window == 9) {
            for (auto& [w, c] : counts) {
                if (w >= 9 && w <= 12) count += c;
            }
            label = "9-12";
        }
        else if (window == 7) {
            for (auto& [w, c] : counts) {
                if (w >= 7 && w <= 8) count += c;
            }
            label = "7-8";
        }
        else if (window == 5) {
            for (auto& [w, c] : counts) {
                if (w >= 5 && w <= 6) count += c;
            }
            label = "5-6";
        }
        else {
            count = counts.count(window) ? counts.at(window) : 0;
            label = std::to_string(window);
        }
    }
    else {
        count = counts.count(window) ? counts.at(window) : 0;
        label = std::to_string(window);
    }

    return label + ": " + std::to_string(count);
}

void RowsOverlay::updateRowLabels() {
    auto& olk = *OlkFPC::get();

    std::map<int, int> counts;
    for (const auto& click : olk.m_clicks) {
        if (click.frameWindow >= 0) {
            counts[click.frameWindow]++;
        }
    }

    auto rows = getConfiguredRows();

    while (m_rowLabels.size() < rows.size()) {
        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({ 0.0f, 0.5f });
        m_rowLabels.push_back(label);
        this->addChild(label);
    }

    for (size_t i = rows.size(); i < m_rowLabels.size(); ++i) {
        m_rowLabels[i]->setVisible(false);
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        int window = rows[i];
        std::string text = getLabelForWindow(window, counts);

        auto label = m_rowLabels[i];
        label->setString(text.c_str());
        label->setColor(getColorForWindow(window));
        label->setPosition(m_baseX, m_baseY - i * m_lineHeight);
        label->setVisible(true);
    }
}
