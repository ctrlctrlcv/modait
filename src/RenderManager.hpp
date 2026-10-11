#pragma once
#include "Types.hpp"
#include <cstdio>
#include <filesystem>

namespace fbfc {

struct RenderPreset { char const* name; int w, h, fps; };

class RenderManager {
public:
    static RenderManager& get() { static RenderManager i; return i; }
    static RenderPreset presetByName(std::string const& n);

    geode::Result<> start(std::filesystem::path const& out);
    void captureFrame();        // llamado tras CCDirector::drawScene
    void finish();
    bool isRendering() const { return m_pipe != nullptr; }
    float fixedDt() const { return 1.f / static_cast<float>(m_preset.fps); }

private:
    FILE* m_pipe = nullptr;
    RenderPreset m_preset{"1080p60", 1920, 1080, 60};
    int m_fw = 0, m_fh = 0;
    std::vector<uint8_t> m_buf, m_flip;
};

}  // namespace fbfc
