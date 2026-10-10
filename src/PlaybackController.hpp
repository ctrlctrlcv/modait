#pragma once
#include <Geode/Geode.hpp>
#include "OlkFPC.hpp"

class PlaybackController {
public:
    PlaybackController();
    ~PlaybackController();
    void startPlayback();
    void stopPlayback();
    void startRecording();
    void stopRecording();
    void onGameFrame(float dt);
    bool isPlaying() const { return m_isPlaying; }
private:
    bool m_isPlaying = false;
    bool m_isRecording = false;
    size_t m_currentClickIndex = 0;
    float m_playbackSpeed = 1.0f;
    float m_recordingSpeed = 1.0f;
    std::vector<ClickData> m_recordedClicks;
};