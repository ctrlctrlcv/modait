#pragma once
#include "Types.hpp"

namespace fbfc {

class PlaybackController {
public:
    enum class Mode { Idle, Recording, Playing };
    static PlaybackController& get() { static PlaybackController i; return i; }

    void startRecording();
    MacroData stopRecording();
    void startPlayback(MacroData macro, float speed = 1.f);
    void stop();

    Mode mode() const { return m_mode; }
    bool isInjecting() const { return m_injecting; }
    void onRecordedEvent(ClickEvent const& e);
    void onReset();
    void inject(GJBaseGameLayer* gl);   // llamado desde GJBaseGameLayer::processCommands

private:
    Mode m_mode = Mode::Idle;
    bool m_injecting = false;
    size_t m_cursor = 0;
    MacroData m_macro, m_recorded;
};

}  // namespace fbfc
