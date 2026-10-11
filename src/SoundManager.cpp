#include "SoundManager.hpp"
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace fbfc {

float SoundManager::pitchFor(int w) {
    if (w <= 0) return 1.5f;
    if (w >= 5) return 0.8f;
    return 1.5f + (0.8f - 1.5f) * (static_cast<float>(w) / 5.f);
}

void SoundManager::setResults(std::vector<ClickResult> const& r, double macroFps) {
    m_triggers.clear();
    auto timing = Mod::get()->getSettingValue<std::string>("sound-timing");
    for (auto& c : r) {
        if (!c.valid()) continue;
        int64_t winMacro = std::llround(c.windowMs * macroFps / 1000.0);
        int64_t at = c.frame;
        if (timing == "Before Click") at = c.frame - winMacro;
        else if (timing == "After Click") at = c.frame + winMacro;
        else if (timing == "Perfect Frame" && c.window != 0) continue;
        m_triggers.emplace(std::max<int64_t>(0, at), c.window);
    }
}

void SoundManager::reset() { m_last = -1; }

void SoundManager::onFrame(int64_t frame) {
    if (!Mod::get()->getSettingValue<bool>("sound-enabled") || m_triggers.empty()) return;
    if (frame < m_last) m_last = -1;
    auto [lo, hi] = std::pair{m_triggers.upper_bound(m_last), m_triggers.upper_bound(frame)};
    for (auto it = lo; it != hi; ++it) play(it->second);
    m_last = frame;
}

void SoundManager::play(int window) {
    auto* mod = Mod::get();
    float vol = static_cast<float>(mod->getSettingValue<double>("sound-volume"));
    float pitch = mod->getSettingValue<bool>("sound-pitch") ? pitchFor(window) : 1.f;

    std::string path = "GJ_button01.ogg";
    if (mod->getSettingValue<bool>("sound-local")) {
        auto local = mod->getResourcesDir() / "bing.mp3";
        if (std::filesystem::exists(local)) path = local.string();
    }
    // Firma según bindings 2.2: (path, speed, pan, volume, pitch, fft, reverb, start, end, fadeIn, fadeOut,
    //                            loop, p12, override, p14, p15, uniqueID, minInterval, sfxGroup)
    FMODAudioEngine::sharedEngine()->playEffectAdvanced(
        path, 1.f, 0.f, vol, pitch, false, false, 0, 0, 0, 0, false, 0, false, false, 0, 0, 0.f, 0);
}

}  // namespace fbfc
