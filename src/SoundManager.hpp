#pragma once
#include "Types.hpp"
#include <map>

namespace fbfc {

class SoundManager {
public:
    static SoundManager& get() { static SoundManager i; return i; }
    void setResults(std::vector<ClickResult> const& r, double macroFps);
    void reset();
    void onFrame(int64_t frame);            // llamado cada frame de juego
    void play(int window);                  // reproduce ahora con el pitch correspondiente
    static float pitchFor(int window);      // 0 -> 1.5, 5+ -> 0.8

private:
    std::multimap<int64_t, int> m_triggers; // frame -> window
    int64_t m_last = -1;
};

}  // namespace fbfc
