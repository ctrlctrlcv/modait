#include "OlkFPC.hpp"
#include "MacroParser.hpp"
#include "FrameAnalyzer.hpp"
#include "PlaybackController.hpp"
#include "RenderManager.hpp"

OlkFPC* OlkFPC::s_instance = nullptr;

OlkFPC* OlkFPC::get() {
    if (!s_instance) s_instance = new OlkFPC();
    return s_instance;
}

void OlkFPC::onModLoaded() {
    m_playbackController = new PlaybackController();
    m_renderManager = new RenderManager();
}

void OlkFPC::onLevelStart() {
    m_clicks.clear();
    m_analysisDone = false;
}

void OlkFPC::onLevelQuit() {
    stopPlayback();
    stopRecording();
}

bool OlkFPC::loadMacro(const std::filesystem::path& path) {
    MacroFormat fmt = MacroParser::detectFormat(path);
    if (fmt == MacroFormat::UNKNOWN) return false;
    
    auto result = MacroParser::parse(path, fmt);
    if (result.isErr()) {
        Notification::create(result.unwrapErr(), NotificationIcon::Error)->show();
        return false;
    }
    
    m_clicks = result.unwrap();
    m_currentFormat = fmt;
    m_macroName = path.filename().string();
    m_macroPath = path.string();
    
    // Calcular duración estimada del nivel (simplificado)
    float duration = 120.0f; // 2 minutos por defecto
    if (!m_clicks.empty()) duration = m_clicks.back().time + 5.0f;
    
    // Analizar
    auto config = FrameAnalyzer::getConfigFromSettings();
    FrameAnalyzer::analyzeAllClicks(m_clicks, config, duration);
    
    // Generar Insights
    std::vector<int> windows = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto dist = FrameAnalyzer::computeDistribution(m_clicks, windows);
    m_insights = FrameAnalyzer::computeInsights(m_clicks, dist);
    
    m_analysisDone = true;
    Notification::create("Macro cargado y analizado", NotificationIcon::Success)->show();
    return true;
}

void OlkFPC::analyzeClicks() {
    if (m_clicks.empty()) return;
    auto config = FrameAnalyzer::getConfigFromSettings();
    float duration = m_clicks.back().time + 5.0f;
    FrameAnalyzer::analyzeAllClicks(m_clicks, config, duration);
    std::vector<int> windows = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto dist = FrameAnalyzer::computeDistribution(m_clicks, windows);
    m_insights = FrameAnalyzer::computeInsights(m_clicks, dist);
    m_analysisDone = true;
}

void OlkFPC::startPlayback() { if (m_playbackController) m_playbackController->startPlayback(); }
void OlkFPC::stopPlayback() { if (m_playbackController) m_playbackController->stopPlayback(); }
void OlkFPC::startRecording() { if (m_playbackController) m_playbackController->startRecording(); }
void OlkFPC::stopRecording() { if (m_playbackController) m_playbackController->stopRecording(); }