#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <matjson.hpp>

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
//  OLKFPC — CLASE PRINCIPAL
// ============================================================

class OlkFPC {
public:
    static OlkFPC* get();

    void onModLoaded();
    void onLevelStart();
    void onLevelQuit();

    bool loadMacro(const std::filesystem::path& path);
    void analyzeClicks();
    void computeFrameWindows();

    void startPlayback();
    void stopPlayback();
    void startRecording();
    void stopRecording();

    void openMainMenu();
    void openSettings();
    void openInsights();

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

private:
    static OlkFPC* s_instance;
    OlkFPC() = default;
};