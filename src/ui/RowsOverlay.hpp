#pragma once
#include <Geode/Geode.hpp>
#include "../OlkFPC.hpp"

using namespace geode::prelude;

// ============================================================
//  ROWSOVERLAY — Filas de conteo en la esquina superior
// ============================================================

class RowsOverlay : public cocos2d::CCNode {
public:
    static RowsOverlay* create();
    bool init() override;
    void update(float dt) override;

    void setOverlayVisible(bool visible) { m_visible = visible; }

    // Forzar actualización manual (útil al cargar un macro)
    void refresh();

private:
    void updateRowLabels();

    // Obtener las filas configuradas según el modo (Short/Normal/Personalize)
    std::vector<int> getConfiguredRows() const;

    // Parsear el string personalizado "13-15,9-12,7-8,5-6,4,3,2,1"
    std::vector<int> parseCustomRows(const std::string& custom) const;

    // Color según frame window
    cocos2d::ccColor3B getColorForWindow(int window) const;

    // Etiqueta del frame window
    std::string getLabelForWindow(int window, const std::map<int, int>& counts) const;

    std::vector<cocos2d::CCLabelBMFont*> m_rowLabels;
    std::vector<cocos2d::CCLabelBMFont*> m_countLabels;
    bool m_visible = true;
    bool m_dirty = true;

    // Posición base
    float m_baseX = 20.0f;
    float m_baseY = 0.0f;
    float m_lineHeight = 24.0f;
};