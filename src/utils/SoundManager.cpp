#include "SoundManager.hpp"
#include <Geode/utils/file.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

SoundManager* SoundManager::s_instance = nullptr;

SoundManager* SoundManager::get() {
    if (!s_instance) s_instance = new SoundManager();
    return s_instance;
}

void SoundManager::init() {
    if (m_initialized) return;

    // Ruta del sonido dentro de los recursos del mod
    m_soundPath = "bing.mp3"_spr;

    auto path = Mod::get()->getResourcesDir() / "bing.mp3";
    if (!std::filesystem::exists(path)) {
        log::warn("OlkFPC: No se encontró bing.mp3 en resources/, usando sonido por defecto");
        m_soundPath = "GJ_button01.ogg";
    }

    auto engine = FMODAudioEngine::sharedEngine();
    if (engine) {
        engine->preloadEffect(m_soundPath);
        log::info("OlkFPC: Sonido '{}' precargado", m_soundPath);
    }

    m_initialized = true;
}

float SoundManager::frameWindowToPitch(int frameWindow) const {
    if (!m_varyPitch) return 1.0f;

    // Frame window 0 → pitch 1.5, 5+ → 0.8
    float pitch = 1.5f - (frameWindow * 0.14f);
    return std::clamp(pitch, 0.8f, 1.5f);
}

void SoundManager::playClickSound(int frameWindow, const std::string& timing) {
    if (!m_initialized) init();

    auto engine = FMODAudioEngine::sharedEngine();
    if (!engine) return;

    if (timing == "Perfect Frame" && frameWindow != 0) {
        return;
    }

    float pitch = frameWindowToPitch(frameWindow);

    // ✅ Firma completa de playEffectAdvanced (19 argumentos):
    // (path, speed, unknown, volume, pitch, fft, reverb,
    //  startMillis, endMillis, fadeIn, fadeOut, loop,
    //  effectID, override, noPreload, channelID, uniqueID,
    //  minInterval, sfxGroup)
    engine->playEffectAdvanced(
        m_soundPath,        // path
        1.0f,               // speed
        1.0f,               // unknown
        m_baseVolume,       // volume
        pitch,              // pitch
        false,              // fastFourierTransform
        false,              // reverb
        0,                  // startMillis
        0,                  // endMillis
        0,                  // fadeIn
        0,                  // fadeOut
        false,              // loopEnabled
        0,                  // effectID
        false,              // override
        false,              // noPreload
        0,                  // channelID
        0,                  // uniqueID
        0.0f,               // minInterval
        0                   // sfxGroup
    );

    log::debug("OlkFPC: Sonido reproducido - FW: {}, Pitch: {:.2f}", frameWindow, pitch);
}

void SoundManager::playCustomSound(const std::string& path, float pitch, float volume) {
    auto engine = FMODAudioEngine::sharedEngine();
    if (!engine) return;

    engine->playEffectAdvanced(
        path,
        1.0f,                                    // speed
        1.0f,                                    // unknown
        std::clamp(volume, 0.0f, 1.0f),          // volume
        std::clamp(pitch, 0.1f, 3.0f),           // pitch
        false,                                   // fastFourierTransform
        false,                                   // reverb
        0, 0, 0, 0,                              // startMillis, endMillis, fadeIn, fadeOut
        false,                                   // loopEnabled
        0,                                       // effectID
        false,                                   // override
        false,                                   // noPreload
        0,                                       // channelID
        0,                                       // uniqueID
        0.0f,                                    // minInterval
        0                                        // sfxGroup
    );
}
