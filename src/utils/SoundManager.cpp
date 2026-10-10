#include "SoundManager.hpp"
#include <Geode/utils/file.hpp>

SoundManager* SoundManager::s_instance = nullptr;

SoundManager* SoundManager::get() {
    if (!s_instance) s_instance = new SoundManager();
    return s_instance;
}

void SoundManager::init() {
    if (m_initialized) return;

    // Ruta del sonido dentro de los recursos del mod
    // En Geode, los recursos se acceden con el sufijo "_spr"
    m_soundPath = "bing.mp3"_spr;

    // Verificar que el archivo existe
    auto path = Mod::get()->getResourcesDir() / "bing.mp3";
    if (!std::filesystem::exists(path)) {
        log::warn("OlkFPC: No se encontró bing.mp3 en resources/, usando sonido por defecto");
        m_soundPath = "GJ_button01.ogg"; // Fallback a un sonido de GD
    }

    // Precargar el sonido en FMOD
    auto engine = FMODAudioEngine::sharedEngine();
    if (engine) {
        engine->preloadEffect(m_soundPath);
        log::info("OlkFPC: Sonido '{}' precargado", m_soundPath);
    }

    m_initialized = true;
}

float SoundManager::frameWindowToPitch(int frameWindow) const {
    if (!m_varyPitch) return 1.0f;

    // Mapeo lineal: frame window 0 → pitch 1.5, 5+ → pitch 0.8
    // Cada frame window reduce el pitch en 0.14
    float pitch = 1.5f - (frameWindow * 0.14f);

    // Clamp para evitar pitches extremos
    return std::clamp(pitch, 0.8f, 1.5f);
}

void SoundManager::playClickSound(int frameWindow, const std::string& timing) {
    if (!m_initialized) init();

    auto engine = FMODAudioEngine::sharedEngine();
    if (!engine) return;

    // Calcular pitch según frame window
    float pitch = frameWindowToPitch(frameWindow);

    // El timing determina cuándo se reproduce el sonido
    // En la práctica, "On Click" es el comportamiento por defecto
    // "Before Click" y "After Click" requieren un pequeño delay
    // "Perfect Frame" solo suena si frameWindow == 0

    if (timing == "Perfect Frame" && frameWindow != 0) {
        return; // No reproducir si no es perfecto
    }

    // Reproducir con FMOD
    // playEffectAdvanced(path, volume, pitch, pan, paused, ...)
    engine->playEffectAdvanced(
        m_soundPath,
        m_baseVolume,       // volumen
        pitch,              // pitch
        0.0f,               // pan (centro)
        1.0f,               // speed
        false,              // paused
        false,              // loop
        0,                  // loopStart
        0,                  // loopEnd
        false,              // forceLoad
        0                   // uniqueID
    );

    log::debug("OlkFPC: Sonido reproducido - FW: {}, Pitch: {:.2f}, Timing: {}",
        frameWindow, pitch, timing);
}

void SoundManager::playCustomSound(const std::string& path, float pitch, float volume) {
    auto engine = FMODAudioEngine::sharedEngine();
    if (!engine) return;

    engine->playEffectAdvanced(
        path,
        std::clamp(volume, 0.0f, 1.0f),
        std::clamp(pitch, 0.1f, 3.0f),
        0.0f, 1.0f, false, false, 0, 0, false, 0
    );
}