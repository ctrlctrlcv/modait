#pragma once
#include "../Types.hpp"

namespace fbfc {

// Filas en la esquina superior izquierda: flujo de inputs y su precisión.
class RowsOverlay : public cocos2d::CCNode {
public:
    static RowsOverlay* attach(PlayLayer* pl);
    void refresh();
    // "0-3,5,9+" -> predicado sobre window
    static std::vector<std::pair<int, int>> parseRanges(std::string const& s);
private:
    size_t m_lastCount = SIZE_MAX;
    uint64_t m_rev = UINT64_MAX;
};

}  // namespace fbfc
