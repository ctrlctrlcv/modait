#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

// ============================================================
//  SOUNDMANAGER — Manejo del sonido BING! con pitch variable
// ============================================================

class SoundManager {
public:
    static SoundManager* get();

    // Inicialización (precarga el sonido)
    void init();

    // Reproducir sonido de click
    // frameWindow: el frame window del click (0 = perfecto, 5+ = fácil)
    // timing: "On Click", "Before Click", "After Click", "Perfect Frame"
    void playClickSound(int frameWindow, const std::string& timing = "On Click");

    // Configuración
    void setPitchVariation(bool enabled) { m_varyPitch = enabled; }
    void setBaseVolume(float volume) { m_baseVolume = std::clamp(volume, 0.0f, 1.0f); }

    // Reproducir sonido personalizado
    void playCustomSound(const std::string& path, float pitch = 1.0f, float volume = 1.0f);

private:
    SoundManager() = default;
    static SoundManager* s_instance;

    bool m_varyPitch = true;
    float m_baseVolume = 1.0f;
    bool m_initialized = false;
    std::string m_soundPath;

    // Mapear frame window a pitch
    // 0 → 1.5, 1 → 1.4, 2 → 1.3, ... 5+ → 0.8
    float frameWindowToPitch(int frameWindow) const;
};