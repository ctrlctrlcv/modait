#pragma once
#include <Geode/Geode.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fbfc {

enum class PatternTag : uint8_t { None, StraightFly, WaveSpam };

// Evento de input crudo capturado en PlayerObject::pushButton / releaseButton.
struct ClickEvent {
    int64_t frame = 0;       // frame exacto (m_gameState.m_currentProgress)
    double timeMs = 0.0;     // marca de tiempo del nivel
    bool down = true;        // true = push, false = release
    bool hold = false;       // true en un release que cierra un mantenimiento
    int64_t heldFrames = 0;  // duración del hold (solo en release)
    bool player2 = false;
    float posX = 0.f, posY = 0.f;
};

struct MacroData {
    double fps = 240.0;
    std::string name;
    std::vector<ClickEvent> events;
    bool empty() const { return events.empty(); }
};

// window: tolerancia en frames de scan = min(back, fwd). -1 = el click falla incluso sin desplazamiento.
struct ClickResult {
    size_t eventIndex = 0;
    int64_t frame = 0;        // frame en unidades de macro-fps
    double timeMs = 0.0;
    int window = -1;
    int back = 0, fwd = 0;
    double windowMs = 0.0;
    bool player2 = false;
    float posX = 0.f, posY = 0.f;
    PatternTag pattern = PatternTag::None;
    bool valid() const { return window >= 0; }
};

struct Insights {
    size_t total = 0, exact = 0;
    int tightest = -1;
    double median = 0.0, clicksPerSec = 0.0;
    std::array<size_t, 8> buckets{};
    std::vector<ClickResult> hardest;  // top 10
};

inline int bucketIndex(int w) {
    if (w <= 4) return w < 0 ? 0 : w;
    if (w <= 6) return 5;
    if (w <= 8) return 6;
    return 7;
}

// 0 Rojo, 1 Naranja, 2 Amarillo, 3 Blanco, 4 Verde claro, 5-6 Cian, 7-8 Azul, 9+ Violeta
inline cocos2d::ccColor3B colorForWindow(int w) {
    static constexpr cocos2d::ccColor3B table[8] = {
        {255, 60, 60}, {255, 150, 40}, {255, 230, 60}, {255, 255, 255},
        {140, 255, 140}, {60, 230, 255}, {60, 110, 255}, {170, 90, 255}};
    if (w < 0) return {120, 120, 120};
    return table[bucketIndex(w)];
}

inline cocos2d::ccColor4F toF(cocos2d::ccColor3B c, float a = 1.f) {
    return {c.r / 255.f, c.g / 255.f, c.b / 255.f, a};
}

inline char const* kBucketLabels[8] = {"0", "1", "2", "3", "4", "5-6", "7-8", "9+"};

}  // namespace fbfc
