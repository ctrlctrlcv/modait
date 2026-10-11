#include "OlkFPC.hpp"
#include "MacroParser.hpp"
#include "FrameAnalyzer.hpp"
#include "PlaybackController.hpp"
#include "RenderManager.hpp"
#include <Geode/utils/file.hpp>

// ============================================================
//  SINGLETON
// ============================================================

OlkFPC* OlkFPC::s_instance = nullptr;

OlkFPC* OlkFPC::get() {
    if (!s_instance) {
        s_instance = new OlkFPC();
    }
    return s_instance;
}

// ============================================================
//  CICLO DE VIDA DEL MOD
// ============================================================

void OlkFPC::onModLoaded() {
    log::info("OlkFPC: Mod cargado");

    // Inicializar componentes
    m_playbackController = new PlaybackController();
    m_renderManager = new RenderManager();

    log::info("OlkFPC: Componentes inicializados");
}

void OlkFPC::onLevelStart() {
    log::info("OlkFPC: Nivel iniciado");

    // Resetear estado del nivel
    m_clicks.clear();
    m_analysisDone = false;
    m_isPlaying = false;
    m_isRecording = false;
    m_insights = InsightsData{};
}

void OlkFPC::onLevelQuit() {
    log::info("OlkFPC: Nivel terminado");

    // Detener reproducción y grabación
    stopPlayback();
    stopRecording();
}

// ============================================================
//  CARGA Y ANÁLISIS DE MACROS
// ============================================================

bool OlkFPC::loadMacro(const std::filesystem::path& path) {
    log::info("OlkFPC: Cargando macro: {}", path.string());

    // Detectar formato
    MacroFormat fmt = MacroParser::detectFormat(path);
    if (fmt == MacroFormat::UNKNOWN) {
        Notification::create(
            "Formato de macro no soportado",
            NotificationIcon::Error
        )->show();
        return false;
    }

    // Parsear el archivo
    auto result = MacroParser::parse(path, fmt);
    if (result.isErr()) {
        Notification::create(
            fmt::format("Error al cargar macro: {}", result.unwrapErr()),
            NotificationIcon::Error
        )->show();
        return false;
    }

    // Guardar datos
    m_clicks = result.unwrap();
    m_currentFormat = fmt;
    m_macroName = path.filename().string();
    m_macroPath = path.string();

    log::info("OlkFPC: {} clicks cargados", m_clicks.size());

    // Calcular duración estimada del nivel
    float duration = 120.0f;
    if (!m_clicks.empty()) {
        duration = m_clicks.back().time + 5.0f;
    }

    // Analizar clicks
    auto config = FrameAnalyzer::getConfigFromSettings();
    FrameAnalyzer::analyzeAllClicks(m_clicks, config, duration);

    // Generar distribución de frame windows
    std::vector<int> windows = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto dist = FrameAnalyzer::computeDistribution(m_clicks, windows);

    // Calcular insights
    m_insights = FrameAnalyzer::computeInsights(m_clicks, dist);

    m_analysisDone = true;

    Notification::create(
        fmt::format("Macro cargado: {} clicks", m_clicks.size()),
        NotificationIcon::Success
    )->show();

    return true;
}

void OlkFPC::analyzeClicks() {
    if (m_clicks.empty()) {
        log::warn("OlkFPC: No hay clicks para analizar");
        return;
    }

    auto config = FrameAnalyzer::getConfigFromSettings();
    float duration = m_clicks.back().time + 5.0f;

    FrameAnalyzer::analyzeAllClicks(m_clicks, config, duration);

    std::vector<int> windows = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto dist = FrameAnalyzer::computeDistribution(m_clicks, windows);
    m_insights = FrameAnalyzer::computeInsights(m_clicks, dist);

    m_analysisDone = true;

    log::info("OlkFPC: Análisis completado");
}

// ============================================================
//  REPRODUCCIÓN
// ============================================================

void OlkFPC::startPlayback() {
    if (!m_playbackController) {
        log::error("OlkFPC: PlaybackController no inicializado");
        return;
    }

    if (m_clicks.empty()) {
        Notification::create(
            "No hay macro cargado",
            NotificationIcon::Warning
        )->show();
        return;
    }

    m_playbackController->startPlayback();
    m_isPlaying = true;

    log::info("OlkFPC: Playback iniciado");
}

void OlkFPC::stopPlayback() {
    if (!m_playbackController) return;

    m_playbackController->stopPlayback();
    m_isPlaying = false;

    log::info("OlkFPC: Playback detenido");
}

// ============================================================
//  GRABACIÓN
// ============================================================

void OlkFPC::startRecording() {
    if (!m_playbackController) {
        log::error("OlkFPC: PlaybackController no inicializado");
        return;
    }

    m_playbackController->startRecording();
    m_isRecording = true;

    log::info("OlkFPC: Grabación iniciada");
}

void OlkFPC::stopRecording() {
    if (!m_playbackController) return;

    m_playbackController->stopRecording();
    m_isRecording = false;

    log::info("OlkFPC: Grabación detenida");
}

// ============================================================
//  UI
// ============================================================

void OlkFPC::openMainMenu() {
    // Esta función es un wrapper para evitar include circular con MainMenuUI
    // La lógica real está en MainMenuUI::create()->show()
    log::info("OlkFPC: Abriendo menú principal");

    // Nota: MainMenuUI se incluye en main.cpp, no aquí, para evitar
    // dependencias circulares. Si quieres llamar a la UI desde aquí,
    // necesitarías incluir MainMenuUI.hpp en este archivo.
}
