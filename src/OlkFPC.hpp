#pragma once
#include "FrameAnalyzer.hpp"
#include "Types.hpp"

namespace fbfc {

// Singleton: estado global, ciclo de vida del nivel y disparadores de análisis.
class OlkFPC {
public:
    static OlkFPC& get() { static OlkFPC i; return i; }

    // --- ciclo de vida (llamado desde los hooks de main.cpp)
    void onLevelEnter(PlayLayer* pl);
    void onLevelReset(PlayLayer* pl);
    void onDeath();
    void onLevelComplete();
    void onLevelExit();
    void onPlayerInput(PlayerObject* po, PlayerButton btn, bool down);

    int64_t currentFrame() const;
    double currentTimeMs() const;

    // --- guardia para ignorar el PlayLayer de simulación
    bool isSimulating() const { return m_simDepth > 0; }
    void pushSim() { ++m_simDepth; }
    void popSim() { --m_simDepth; }

    // --- datos
    MacroData const& liveRun() const { return m_live; }
    MacroData const& lastRun() const { return m_last; }
    MacroData const& analyzedMacro() const { return m_analyzed; }
    bool held(bool p2) const { return m_held[p2]; }
    GJGameLevel* level() const { return m_level; }

    // --- análisis
    geode::Result<> requestAnalysis();                         // última run (o la actual)
    geode::Result<> requestAnalysis(MacroData macro);          // macro importada
    void cancelAnalysis() { m_analyzer.cancel(); }
    bool isAnalyzing() const { return m_analyzer.running(); }
    float analysisProgress() const { return m_analyzer.progress(); }
    std::vector<ClickResult> const& results() const { return m_results; }
    Insights const& insights() const { return m_insights; }
    uint64_t revision() const { return m_revision; }

    void tick();   // llamado cada frame (hook de CCScheduler::update)

private:
    void finishAnalysis();

    geode::Ref<GJGameLevel> m_level;
    MacroData m_live, m_last, m_analyzed;
    bool m_held[2] = {false, false};
    int64_t m_pressFrame[2] = {0, 0};
    int m_simDepth = 0;

    FrameAnalyzer m_analyzer;
    std::vector<ClickResult> m_results;
    Insights m_insights;
    uint64_t m_revision = 0;
};

}  // namespace fbfc
