#include "RenderManager.hpp"
#include <Geode/cocos/platform/CCGL.h>
#include <algorithm>
#include <cstring>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
#define FB_POPEN _popen
#define FB_PCLOSE _pclose
#else
#define FB_POPEN popen
#define FB_PCLOSE pclose
#endif

namespace fbfc {

RenderPreset RenderManager::presetByName(std::string const& n) {
    static constexpr RenderPreset presets[] = {
        {"720p30", 1280, 720, 30}, {"720p60", 1280, 720, 60}, {"1080p30", 1920, 1080, 30},
        {"1080p60", 1920, 1080, 60}, {"1440p60", 2560, 1440, 60}, {"4K30", 3840, 2160, 30}, {"4K60", 3840, 2160, 60}};
    for (auto& p : presets) if (n == p.name) return p;
    return presets[3];
}

Result<> RenderManager::start(std::filesystem::path const& out) {
#if defined(GEODE_IS_IOS)
    return Err("Video rendering via FFmpeg pipe is not available on iOS (sandbox).");
#else
    if (m_pipe) return Err("Already rendering");
    auto* mod = Mod::get();
    m_preset = presetByName(mod->getSettingValue<std::string>("render-preset"));
    auto size = CCEGLView::get()->getFrameSize();
    m_fw = static_cast<int>(size.width) & ~1; m_fh = static_cast<int>(size.height) & ~1;
    auto ff = mod->getSettingValue<std::string>("ffmpeg-path");
    auto crf = mod->getSettingValue<int64_t>("render-crf");
    std::string cmd = fmt::format(
        "\"{}\" -y -loglevel error -f rawvideo -pix_fmt rgb24 -s {}x{} -framerate {} -i - "
        "-vf scale={}:{}:flags=bicubic -c:v libx264 -preset medium -crf {} -pix_fmt yuv420p \"{}\"",
        ff, m_fw, m_fh, m_preset.fps, m_preset.w, m_preset.h, crf, out.string());
    m_pipe = FB_POPEN(cmd.c_str(), "wb");
    if (!m_pipe) return Err("Could not launch FFmpeg ('{}'). Set 'ffmpeg-path' in settings.", ff);
    m_buf.resize(static_cast<size_t>(m_fw) * m_fh * 3);
    m_flip.resize(m_buf.size());
    return Ok();
#endif
}

void RenderManager::captureFrame() {
    if (!m_pipe) return;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, m_fw, m_fh, GL_RGB, GL_UNSIGNED_BYTE, m_buf.data());
    size_t row = static_cast<size_t>(m_fw) * 3;                 // OpenGL entrega la imagen invertida en vertical
    for (int y = 0; y < m_fh; ++y)
        std::memcpy(&m_flip[static_cast<size_t>(y) * row], &m_buf[static_cast<size_t>(m_fh - 1 - y) * row], row);
    if (std::fwrite(m_flip.data(), 1, m_flip.size(), m_pipe) != m_flip.size()) {
        log::error("FFmpeg pipe closed unexpectedly");
        finish();
    }
}

void RenderManager::finish() {
    if (!m_pipe) return;
    std::fflush(m_pipe);
    FB_PCLOSE(m_pipe);
    m_pipe = nullptr;
    m_buf.clear(); m_flip.clear();
}

}  // namespace fbfc
