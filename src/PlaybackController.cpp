#include "PlaybackController.hpp"

PlaybackController::PlaybackController() = default;
PlaybackController::~PlaybackController() = default;

void PlaybackController::startPlayback() {
    float speed = static_cast<float>(Mod::get()->getSettingValue<double>("playback-speed")) / 100.0f;
    m_playbackSpeed = std::clamp(speed, 0.2f, 5.0f);
    m_currentClickIndex = 0; m_isPlaying = true;
}

void PlaybackController::stopPlayback() { m_isPlaying = false; m_currentClickIndex = 0; }

void PlaybackController::startRecording() {
    float speed = static_cast<float>(Mod::get()->getSettingValue<double>("recording-speed")) / 100.0f;
    m_recordingSpeed = std::clamp(speed, 0.2f, 5.0f);
    m_recordedClicks.clear(); m_isRecording = true;
}

void PlaybackController::stopRecording() {
    m_isRecording = false;
    OlkFPC::get()->m_clicks = m_recordedClicks;
    OlkFPC::get()->m_analysisDone = true;
}

void PlaybackController::onGameFrame(float dt) {
    auto& olk = *OlkFPC::get();
    if (m_isPlaying && !olk.m_clicks.empty()) {
        auto playLayer = PlayLayer::get();
        if (!playLayer) return;
        int currentFrame = static_cast<int>(playLayer->m_gameState.m_levelTime * 240.0f);
        while (m_currentClickIndex < olk.m_clicks.size()) {
            auto& click = olk.m_clicks[m_currentClickIndex];
            int targetFrame = static_cast<int>(click.frame / m_playbackSpeed);
            if (targetFrame <= currentFrame) {
                if (playLayer->m_player1) {
                    if (click.isHold) playLayer->m_player1->pushButton(PlayerButton::Jump);
                    else playLayer->m_player1->releaseButton(PlayerButton::Jump);
                }
                m_currentClickIndex++;
            } else break;
        }
    }
}