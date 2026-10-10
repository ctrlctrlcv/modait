#include "RowsOverlay.hpp"
#include <Geode/utils/string.hpp>
#include <algorithm>

// ============================================================
//  CREACIÓN
// ============================================================

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

    // Configurar posición base (esquina superior izquierda)
    auto winSize = CCDirector::get()->getWinSize();
    m_baseX = 20.0f;
    m_baseY = winSize.height - 40.0f;

    this->setPosition(0, 0);
    this->scheduleUpdate();
    return true;
}

// ============================================================
//  UPDATE PRINCIPAL
// ============================================================

void RowsOverlay::update(float dt) {
    if (!m_visible) {
        this->setVisible(false);
        return;
    }
    this->setVisible(true);

    // Solo actualizar si el análisis está listo y hay cambios
    auto& olk = *OlkFPC::get();
    if (!olk.m_analysisDone) return;

    // Actualizar cada frame para reflejar cambios en tiempo real
    updateRowLabels();
}

void RowsOverlay::refresh() {
    m_dirty = true;
    updateRowLabels();
}

// ============================================================
//  OBTENER FILAS CONFIGURADAS
// ============================================================

std::vector<int> RowsOverlay::getConfiguredRows() const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    if (mode == "Short") {
        // 1, 2, 3, 4, 5+ (donde 5+ agrupa 5,6,7,8,9+)
        return { 1, 2, 3, 4, 5 };
    }

    if (mode == "Normal") {
        // Orden de arriba a abajo: 13-15, 9-12, 7-8, 5-6, 4, 3, 2, 1
        // Los agrupados se representan por su valor mínimo
        return { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    // Personalize
    std::string custom = mod->getSettingValue<std::string>("rows-custom");
    return parseCustomRows(custom);
}

std::vector<int> RowsOverlay::parseCustomRows(const std::string& custom) const {
    std::vector<int> rows;

    auto parts = geode::utils::string::split(custom, ",");
    for (auto& part : parts) {
        // Trim espacios
        part = geode::utils::string::trim(part);
        if (part.empty()) continue;

        if (part.find('-') != std::string::npos) {
            // Rango: "13-15"
            auto range = geode::utils::string::split(part, "-");
            if (range.size() == 2) {
                try {
                    int start = std::stoi(geode::utils::string::trim(range[0]));
                    int end = std::stoi(geode::utils::string::trim(range[1]));
                    // El rango se representa por su valor mínimo
                    rows.push_back(start);
                } catch (...) {
                    log::warn("OlkFPC: Rango inválido: {}", part);
                }
            }
        } else {
            // Valor individual
            try {
                rows.push_back(std::stoi(part));
            } catch (...) {
                log::warn("OlkFPC: Valor inválido: {}", part);
            }
        }
    }

    if (rows.empty()) {
        // Fallback a Normal
        rows = { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    return rows;
}

// ============================================================
//  COLORES Y ETIQUETAS
// ============================================================

cocos2d::ccColor3B RowsOverlay::getColorForWindow(int window) const {
    // Mismo esquema de colores que los círculos
    if (window == 0)      return { 255, 100, 100 };  // Rojo
    if (window == 1)      return { 255, 165, 0 };    // Naranja
    if (window == 2)      return { 255, 255, 100 };  // Amarillo
    if (window == 3)      return { 255, 255, 255 };  // Blanco
    if (window == 4)      return { 150, 255, 150 };  // Verde claro
    if (window <= 6)      return { 100, 255, 255 };  // Cian
    if (window <= 8)      return { 100, 200, 255 };  // Azul
    return { 200, 150, 255 };                        // Púrpura
}

std::string RowsOverlay::getLabelForWindow(int window,
                                            const std::map<int, int>& counts) const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    int count = 0;
    std::string label;

    if (mode == "Short" && window == 5) {
        // 5+ agrupa 5, 6, 7, 8, 9+
        for (auto& [w, c] : counts) {
            if (w >= 5) count += c;
        }
        label = "5+";
    }
    else if (mode == "Normal") {
        // Agrupaciones del modo Normal
        if (window == 13) {
            // 13-15
            for (auto& [w, c] : counts) {
                if (w >= 13 && w <= 15) count += c;
            }
            label = "13-15";
        }
        else if (window == 9) {
            // 9-12
            for (auto& [w, c] : counts) {
                if (w >= 9 && w <= 12) count += c;
            }
            label = "9-12";
        }
        else if (window == 7) {
            // 7-8
            for (auto& [w, c] : counts) {
                if (w >= 7 && w <= 8) count += c;
            }
            label = "7-8";
        }
        else if (window == 5) {
            // 5-6
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
        // Personalize: cada fila es un valor individual o el mínimo de un rango
        count = counts.count(window) ? counts.at(window) : 0;
        label = std::to_string(window);
    }

    return label + ": " + std::to_string(count);
}

// ============================================================
//  ACTUALIZACIÓN DE ETIQUETAS
// ============================================================

void RowsOverlay::updateRowLabels() {
    auto& olk = *OlkFPC::get();

    // Contar clicks por frame window
    std::map<int, int> counts;
    for (const auto& click : olk.m_clicks) {
        if (click.frameWindow >= 0) {
            counts[click.frameWindow]++;
        }
    }

    auto rows = getConfiguredRows();

    // Reutilizar etiquetas existentes o crear nuevas
    while (m_rowLabels.size() < rows.size()) {
        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({ 0.0f, 0.5f });
        m_rowLabels.push_back(label);
        this->addChild(label);
    }

    // Ocultar etiquetas sobrantes
    for (size_t i = rows.size(); i < m_rowLabels.size(); ++i) {
        m_rowLabels[i]->setVisible(false);
    }

    // Actualizar cada etiqueta
    for (size_t i = 0; i < rows.size(); ++i) {
        int window = rows[i];

        std::string text = getLabelForWindow(window, counts);

        auto label = m_rowLabels[i];
        label->setString(text.c_str());
        label->setColor(getColorForWindow(window));
        label->setPosition(m_baseX, m_baseY - i * m_lineHeight);
        label->setVisible(true);
    }
}#include "RowsOverlay.hpp"
#include <Geode/utils/string.hpp>
#include <algorithm>

// ============================================================
//  CREACIÓN
// ============================================================

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

    // Configurar posición base (esquina superior izquierda)
    auto winSize = CCDirector::get()->getWinSize();
    m_baseX = 20.0f;
    m_baseY = winSize.height - 40.0f;

    this->setPosition(0, 0);
    this->scheduleUpdate();
    return true;
}

// ============================================================
//  UPDATE PRINCIPAL
// ============================================================

void RowsOverlay::update(float dt) {
    if (!m_visible) {
        this->setVisible(false);
        return;
    }
    this->setVisible(true);

    // Solo actualizar si el análisis está listo y hay cambios
    auto& olk = *OlkFPC::get();
    if (!olk.m_analysisDone) return;

    // Actualizar cada frame para reflejar cambios en tiempo real
    updateRowLabels();
}

void RowsOverlay::refresh() {
    m_dirty = true;
    updateRowLabels();
}

// ============================================================
//  OBTENER FILAS CONFIGURADAS
// ============================================================

std::vector<int> RowsOverlay::getConfiguredRows() const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    if (mode == "Short") {
        // 1, 2, 3, 4, 5+ (donde 5+ agrupa 5,6,7,8,9+)
        return { 1, 2, 3, 4, 5 };
    }

    if (mode == "Normal") {
        // Orden de arriba a abajo: 13-15, 9-12, 7-8, 5-6, 4, 3, 2, 1
        // Los agrupados se representan por su valor mínimo
        return { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    // Personalize
    std::string custom = mod->getSettingValue<std::string>("rows-custom");
    return parseCustomRows(custom);
}

std::vector<int> RowsOverlay::parseCustomRows(const std::string& custom) const {
    std::vector<int> rows;

    auto parts = geode::utils::string::split(custom, ",");
    for (auto& part : parts) {
        // Trim espacios
        part = geode::utils::string::trim(part);
        if (part.empty()) continue;

        if (part.find('-') != std::string::npos) {
            // Rango: "13-15"
            auto range = geode::utils::string::split(part, "-");
            if (range.size() == 2) {
                try {
                    int start = std::stoi(geode::utils::string::trim(range[0]));
                    int end = std::stoi(geode::utils::string::trim(range[1]));
                    // El rango se representa por su valor mínimo
                    rows.push_back(start);
                } catch (...) {
                    log::warn("OlkFPC: Rango inválido: {}", part);
                }
            }
        } else {
            // Valor individual
            try {
                rows.push_back(std::stoi(part));
            } catch (...) {
                log::warn("OlkFPC: Valor inválido: {}", part);
            }
        }
    }

    if (rows.empty()) {
        // Fallback a Normal
        rows = { 13, 9, 7, 5, 4, 3, 2, 1 };
    }

    return rows;
}

// ============================================================
//  COLORES Y ETIQUETAS
// ============================================================

cocos2d::ccColor3B RowsOverlay::getColorForWindow(int window) const {
    // Mismo esquema de colores que los círculos
    if (window == 0)      return { 255, 100, 100 };  // Rojo
    if (window == 1)      return { 255, 165, 0 };    // Naranja
    if (window == 2)      return { 255, 255, 100 };  // Amarillo
    if (window == 3)      return { 255, 255, 255 };  // Blanco
    if (window == 4)      return { 150, 255, 150 };  // Verde claro
    if (window <= 6)      return { 100, 255, 255 };  // Cian
    if (window <= 8)      return { 100, 200, 255 };  // Azul
    return { 200, 150, 255 };                        // Púrpura
}

std::string RowsOverlay::getLabelForWindow(int window,
                                            const std::map<int, int>& counts) const {
    auto mod = Mod::get();
    std::string mode = mod->getSettingValue<std::string>("rows-mode");

    int count = 0;
    std::string label;

    if (mode == "Short" && window == 5) {
        // 5+ agrupa 5, 6, 7, 8, 9+
        for (auto& [w, c] : counts) {
            if (w >= 5) count += c;
        }
        label = "5+";
    }
    else if (mode == "Normal") {
        // Agrupaciones del modo Normal
        if (window == 13) {
            // 13-15
            for (auto& [w, c] : counts) {
                if (w >= 13 && w <= 15) count += c;
            }
            label = "13-15";
        }
        else if (window == 9) {
            // 9-12
            for (auto& [w, c] : counts) {
                if (w >= 9 && w <= 12) count += c;
            }
            label = "9-12";
        }
        else if (window == 7) {
            // 7-8
            for (auto& [w, c] : counts) {
                if (w >= 7 && w <= 8) count += c;
            }
            label = "7-8";
        }
        else if (window == 5) {
            // 5-6
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
        // Personalize: cada fila es un valor individual o el mínimo de un rango
        count = counts.count(window) ? counts.at(window) : 0;
        label = std::to_string(window);
    }

    return label + ": " + std::to_string(count);
}

// ============================================================
//  ACTUALIZACIÓN DE ETIQUETAS
// ============================================================

void RowsOverlay::updateRowLabels() {
    auto& olk = *OlkFPC::get();

    // Contar clicks por frame window
    std::map<int, int> counts;
    for (const auto& click : olk.m_clicks) {
        if (click.frameWindow >= 0) {
            counts[click.frameWindow]++;
        }
    }

    auto rows = getConfiguredRows();

    // Reutilizar etiquetas existentes o crear nuevas
    while (m_rowLabels.size() < rows.size()) {
        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({ 0.0f, 0.5f });
        m_rowLabels.push_back(label);
        this->addChild(label);
    }

    // Ocultar etiquetas sobrantes
    for (size_t i = rows.size(); i < m_rowLabels.size(); ++i) {
        m_rowLabels[i]->setVisible(false);
    }

    // Actualizar cada etiqueta
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