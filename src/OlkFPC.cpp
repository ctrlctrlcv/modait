#include "OlkFPC.hpp"
#include "PlaybackController.hpp"
#include "SoundManager.hpp"

using namespace geode::prelude;

namespace fbfc {

static double macroFps() { return static_cast<double>(Mod::get()->getSettingValue<int64_t>("macro-fps")); }

int64_t OlkFPC::currentFrame() const {
    auto* pl = PlayLayer::get();
    return pl ? pl->m_gameState.m_currentProgress : 0;     // un solo punto a cambiar si cambia el binding
}
double OlkFPC::currentTimeMs() const {
    auto* pl = PlayLayer::get();
    return pl ? pl->m_gameState.m_levelTime * 1000.0 : 0.0;
}

void OlkFPC::onLevelEnter(PlayLayer* pl) {
    m_level = pl->m_level;
    m_live = {}; m_live.fps = macroFps();
    m_held[0] = m_held[1] = false;
    SoundManager::get().reset();
}

void OlkFPC::onLevelReset(PlayLayer*) {
    if (m_live.events.size() > 1) m_last = m_live;      // conserva el intento anterior
    m_live = {}; m_live.fps = macroFps();
    m_held[0] = m_held[1] = false;
    PlaybackController::get().onReset();
    SoundManager::get().reset();
}

void OlkFPC::onDeath() { if (!m_live.empty()) m_last = m_live; }

void OlkFPC::onLevelComplete() {
    if (!m_live.empty()) m_last = m_live;
    if (Mod::get()->getSettingValue<bool>("scan-auto")) (void)requestAnalysis();
}

void OlkFPC::onLevelExit() { m_analyzer.cancel(); m_level = nullptr; }

void OlkFPC::onPlayerInput(PlayerObject* po, PlayerButton, bool down) {
    auto* pl = PlayLayer::get();
    if (!pl || isSimulating() || PlaybackController::get().isInjecting()) return;
    bool p2;
    if (po == pl->m_player1) p2 = false; else if (po == pl->m_player2) p2 = true; else return;

    ClickEvent e;
    e.frame = currentFrame(); e.timeMs = currentTimeMs(); e.down = down; e.player2 = p2;
    e.posX = po->getPositionX(); e.posY = po->getPositionY();
    if (down) { m_pressFrame[p2] = e.frame; }
    else { e.hold = true; e.heldFrames = e.frame - m_pressFrame[p2]; }
    m_held[p2] = down;

    if (Mod::get()->getSettingValue<bool>("record-auto")) m_live.events.push_back(e);
    PlaybackController::get().onRecordedEvent(e);
}

Result<> OlkFPC::requestAnalysis() {
    if (!m_last.empty()) return requestAnalysis(m_last);
    if (!m_live.empty()) return requestAnalysis(m_live);
    return Err("No recorded inputs yet. Play the level or import a macro.");
}

Result<> OlkFPC::requestAnalysis(MacroData macro) {
    if (!m_level) return Err("Enter a level first");
    m_analyzed = macro;
    m_results.clear(); m_insights = {};
    ++m_revision;
    return m_analyzer.begin(m_level, macro);
}

void OlkFPC::tick() {
    if (!m_analyzer.running()) return;
    double budget = static_cast<double>(Mod::get()->getSettingValue<int64_t>("scan-budget-ms"));
    if (m_analyzer.step(budget)) finishAnalysis();
    else ++m_revision;                                   // refresca UI de progreso
}

void OlkFPC::finishAnalysis() {
    m_results = m_analyzer.takeResults();
    m_insights = FrameAnalyzer::computeInsights(m_results, m_analyzed.fps);
    SoundManager::get().setResults(m_results, m_analyzed.fps);
    ++m_revision;
}

}  // namespace fbfc
