#pragma once
#include <Geode/Geode.hpp>
#include "OlkFPC.hpp"
#include <cstdio>

class RenderManager {
public:
    RenderManager();
    ~RenderManager();

    void startRender();
    void stopRender();
    void captureFrame();

    void setOutputPath(const std::filesystem::path& path) { m_outputPath = path; }
    void setPreset(const std::string& preset) { m_preset = preset; }

private:
    std::filesystem::path m_outputPath;
    std::string m_preset = "1080p60";
    bool m_isRendering = false;
    int m_frameCount = 0;
    FILE* m_ffmpegPipe = nullptr;
    int m_width = 1920;
    int m_height = 1080;

    std::string getFFmpegArgs() const;
};