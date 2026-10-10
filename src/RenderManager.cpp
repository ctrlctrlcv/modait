#include "RenderManager.hpp"
#include <Geode/utils/file.hpp>
#include <Geode/utils/web.hpp>

RenderManager::RenderManager() = default;
RenderManager::~RenderManager() { stopRender(); }

std::string RenderManager::getFFmpegArgs() const {
    if (m_preset == "720p30") return "-vf scale=1280:720 -r 30 -c:v libx264 -preset fast -crf 23";
    if (m_preset == "720p60") return "-vf scale=1280:720 -r 60 -c:v libx264 -preset fast -crf 23";
    if (m_preset == "1080p30") return "-vf scale=1920:1080 -r 30 -c:v libx264 -preset fast -crf 20";
    if (m_preset == "1080p60") return "-vf scale=1920:1080 -r 60 -c:v libx264 -preset fast -crf 20";
    if (m_preset == "1440p60") return "-vf scale=2560:1440 -r 60 -c:v libx264 -preset medium -crf 18";
    if (m_preset == "4K30") return "-vf scale=3840:2160 -r 30 -c:v libx264 -preset slow -crf 16";
    if (m_preset == "4K60") return "-vf scale=3840:2160 -r 60 -c:v libx264 -preset slow -crf 16";
    return "-vf scale=1920:1080 -r 60 -c:v libx264 -preset fast -crf 20";
}

void RenderManager::startRender() {
    if (m_outputPath.empty()) m_outputPath = Mod::get()->getTempDir() / "olkfpc_render.mp4";
    m_preset = Mod::get()->getSettingValue<std::string>("render-preset");
    m_isRendering = true;
    m_frameCount = 0;

    auto glView = CCDirector::get()->getOpenGLView();
    m_width = glView->getFrameSize().width;
    m_height = glView->getFrameSize().height;

    // Abrir tubería a FFmpeg para inyectar frames raw
    std::string cmd = fmt::format("ffmpeg -y -f rawvideo -pix_fmt rgba -s {}x{} -r {} -i - {} {}",
        m_width, m_height, 240, getFFmpegArgs(), m_outputPath.string());

    m_ffmpegPipe = popen(cmd.c_str(), "w");
    if (!m_ffmpegPipe) {
        Notification::create("Error al iniciar FFmpeg", NotificationIcon::Error)->show();
        m_isRendering = false;
    } else {
        Notification::create("Renderizado iniciado", NotificationIcon::Info)->show();
    }
}

void RenderManager::captureFrame() {
    if (!m_isRendering || !m_ffmpegPipe) return;
    
    // Capturar el framebuffer real del juego
    std::vector<uint8_t> pixels(m_width * m_height * 4);
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Voltear verticalmente (OpenGL lee de abajo hacia arriba)
    std::vector<uint8_t> flipped(m_width * m_height * 4);
    for (int y = 0; y < m_height; ++y) {
        std::memcpy(&flipped[y * m_width * 4], &pixels[(m_height - 1 - y) * m_width * 4], m_width * 4);
    }
    
    fwrite(flipped.data(), 1, flipped.size(), m_ffmpegPipe);
    m_frameCount++;
}

void RenderManager::stopRender() {
    if (m_ffmpegPipe) { pclose(m_ffmpegPipe); m_ffmpegPipe = nullptr; }
    if (m_isRendering) {
        Notification::create(fmt::format("Renderizado completo: {} frames", m_frameCount), NotificationIcon::Success)->show();
        m_isRendering = false;
    }
}