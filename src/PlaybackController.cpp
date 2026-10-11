#include "PlaybackController.hpp"
#include <cmath>

using namespace geode::prelude;

namespace fbfc {

void PlaybackController::startRecording() {
    m_recorded = {};
    m_recorded.fps = static_cast<double>(Mod::get()->getSettingValue<int64_t>("macro-fps"));
    m_mode = Mode::Recording;
}

MacroData PlaybackController::stopRecording() {
    m_mode = Mode::Idle;
    return std::move(m_recorded);
}

void PlaybackController::startPlayback(MacroData macro, float speed) {
    m_macro = std::move(macro);
    m_cursor = 0;
    m_mode = Mode::Playing;
    CCDirector::get()->getScheduler()->setTimeScale(std::max(0.05f, speed));
}

void PlaybackController::stop() {
    if (m_mode == Mode::Playing) CCDirector::get()->getScheduler()->setTimeScale(1.f);
    m_mode = Mode::Idle;
}

void PlaybackController::onRecordedEvent(ClickEvent const& e) {
    if (m_mode == Mode::Recording) m_recorded.events.push_back(e);
}

void PlaybackController::onReset() {
    m_cursor = 0;
    if (m_mode == Mode::Recording) m_recorded.events.clear();   // se graba el intento vigente
}

void PlaybackController::inject(GJBaseGameLayer* gl) {
    if (m_mode != Mode::Playing || m_cursor >= m_macro.events.size()) return;
    int64_t frame = gl->m_gameState.m_currentProgress;
    double scale = static_cast<double>(Mod::get()->getSettingValue<int64_t>("macro-fps")) / m_macro.fps;
    m_injecting = true;
    while (m_cursor < m_macro.events.size()) {
        auto const& e = m_macro.events[m_cursor];
        if (static_cast<int64_t>(std::llround(e.frame * scale)) > frame) break;
        gl->handleButton(e.down, 1, !e.player2);
        ++m_cursor;
    }
    m_injecting = false;
}

}  // namespace fbfc
