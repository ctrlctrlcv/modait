#pragma once
#include "Types.hpp"

namespace fbfc {

// Simulación aislada: PlayLayer temporal que nunca se añade a la escena.
class HeadlessSim {
public:
    ~HeadlessSim() { destroy(); }
    geode::Result<> create(GJGameLevel* level, double fps);
    void destroy();
    void input(bool down, bool player2);
    void step();
    bool dead() const;
    bool completed() const;
    geode::Ref<CheckpointObject> snapshot();
    void restore(CheckpointObject* cp);
private:
    PlayLayer* m_layer = nullptr;
    float m_dt = 1.f / 240.f;
};

class FrameAnalyzer {
public:
    geode::Result<> begin(GJGameLevel* level, MacroData const& macro);
    bool step(double budgetMs);              // true cuando termina
    float progress() const;
    bool running() const { return m_running; }
    void cancel();
    std::vector<ClickResult> takeResults() { return std::move(m_results); }

    static void tagPatterns(std::vector<ClickResult>& r);
    static Insights computeInsights(std::vector<ClickResult> const& r, double macroFps);

private:
    struct In { int64_t frame; bool down; bool p2; size_t srcIndex; };
    enum class Phase { Advance, Check0, Back, Fwd, Next };

    bool runVariant(int shift);
    void applyAndStepBaseline();
    int64_t effFrame(size_t k, int shift) const { return m_in[k].frame + (k == m_curIn ? shift : 0); }
    void finishAll();

    HeadlessSim m_sim;
    MacroData m_macro;
    std::vector<In> m_in;
    std::vector<size_t> m_clicks;   // índices en m_in (solo pushes)
    std::vector<ClickResult> m_results;
    double m_scanFps = 240, m_ratio = 1;
    int m_maxShift = 12, m_horizon = 240;
    bool m_running = false;

    Phase m_phase = Phase::Advance;
    size_t m_idx = 0, m_curIn = 0, m_cursor = 0, m_snapCursor = 0;
    int64_t m_baseFrame = 0, m_snapFrame = 0;
    int m_shift = 1, m_back = 0, m_fwd = 0;
    geode::Ref<CheckpointObject> m_snap;
};

}  // namespace fbfc
