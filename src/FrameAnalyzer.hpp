#pragma once
#include "OlkFPC.hpp"

class FrameAnalyzer {
public:
    struct ScanConfig {
        int framesEachSide = 25;
        float fromPercent = 0.0f;
        float toPercent = 100.0f;
        std::string speed = "FAST";
        bool localCheck = true;
    };

    static ScanConfig getConfigFromSettings();
    static void analyzeAllClicks(std::vector<ClickData>& clicks, const ScanConfig& config, float levelDuration);
    static std::vector<FrameWindowStats> computeDistribution(const std::vector<ClickData>& clicks, const std::vector<int>& customWindows);
    static std::vector<HardestClick> findHardestClicks(const std::vector<ClickData>& clicks, int count = 10);
    static void detectSpecialClicks(std::vector<ClickData>& clicks);
    static InsightsData computeInsights(const std::vector<ClickData>& clicks, const std::vector<FrameWindowStats>& dist);

private:
    static bool simulateClickSurvival(int clickFrame, int windowOffset, float fps, const std::vector<ClickData>& allClicks, const std::string& levelData);
    static int calculateFrameWindow(const ClickData& click, const std::vector<ClickData>& allClicks, const ScanConfig& config, float fps, const std::string& levelData);
};