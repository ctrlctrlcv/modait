#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

// ============================================================
//  ESTRUCTURAS DE DATOS
// ============================================================

enum class MacroFormat { UNKNOWN, CML, TTR3, GDR, GDR2, JSON, GDR_JSON };

struct ClickData {
    int frame = 0;
    float time = 0.0f;
    bool isHold = false;
    bool player2 = false;
    int frameWindow = -1;
    bool isStraightFly = false;
    bool isWaveSpam = false;
    int internalClickIndex = 0;
};

struct FrameWindowStats {
    int window = 0;
    int count = 0;
    float percentage = 0.0f;
    std::vector<int> clickIndices;
};

struct HardestClick {
    int frameWindow;
    int clickIndex;
    float percentage;
    std::string label;
};

struct InsightsData {
    int exact = 0;
    int totalClicks = 0;
    int tightest = -1;
    int median = 0;
    int busiestSecond = 0;
    std::vector<FrameWindowStats> frameWindowDistribution;
    std::vector<HardestClick> hardestClicks;
};

// ============================================================
//  FORWARD DECLARATIONS (para evitar includes circulares)
// ============================================================

class PlaybackController;
class RenderManager;

// ============================================================
//  OLKFPC — CLASE PRINCIPAL
// ============================================================

class OlkFPC {
public:
    static OlkFPC* get();

    // Ciclo de vida
    void onModLoaded();
    void onLevelStart();
    void onLevelQuit();

    // Análisis
    bool loadMacro(const std::filesystem::path& path);
    void analyzeClicks();

    // Reproducción
    void startPlayback();
    void stopPlayback();
    void startRecording();
    void stopRecording();

    // UI
    void openMainMenu();

    // ============================================================
    //  DATOS PÚBLICOS (accesibles desde todos los componentes)
    // ============================================================
    std::vector<ClickData> m_clicks;
    InsightsData m_insights;
    MacroFormat m_currentFormat = MacroFormat::UNKNOWN;
    std::string m_macroName;
    std::string m_levelName;
    std::string m_levelID;
    std::string m_levelData;
    std::string m_macroPath;

    bool m_isPlaying = false;
    bool m_isRecording = false;
    bool m_isRendering = false;
    bool m_analysisDone = false;

    // Componentes (declarados como punteros, forward-declared)
    PlaybackController* m_playbackController = nullptr;
    RenderManager* m_renderManager = nullptr;

private:
    static OlkFPC* s_instance;
    OlkFPC() = default;
    ~OlkFPC() = default;
};
