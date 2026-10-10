#include "FrameAnalyzer.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

// ============================================================
//  SIMULACIÓN FÍSICA REAL MEDIANTE PLAYLAYER HEADLESS
// ============================================================

bool FrameAnalyzer::simulateClickSurvival(int clickFrame, int windowOffset, float fps, const std::vector<ClickData>& allClicks, const std::string& levelData) {
    // Crear un PlayLayer aislado para simular la física exacta del juego
    auto level = GJGameLevel::create();
    level->m_levelString = levelData;
    
    auto playLayer = PlayLayer::create(level, false, false);
    if (!playLayer) return false;
    
    playLayer->setVisible(false); // Modo Headless
    playLayer->m_isPracticeMode = false;
    
    // Ejecutar la simulación frame por frame
    int targetFrame = clickFrame + windowOffset;
    int currentFrame = 0;
    
    // Ordenar clicks por frame para aplicar en orden
    std::vector<ClickData> sortedClicks = allClicks;
    std::sort(sortedClicks.begin(), sortedClicks.end(), [](const ClickData& a, const ClickData& b) {
        return a.frame < b.frame;
    });

    for (const auto& c : sortedClicks) {
        if (c.frame > targetFrame) break;
        
        // Avanzar la física hasta el frame del click
        while (currentFrame < c.frame) {
            playLayer->update(1.0f / fps);
            currentFrame++;
            
            // Si muere antes de llegar al click, este click es inválido
            if (playLayer->m_player1->m_isDead || (playLayer->m_player2 && playLayer->m_player2->m_isDead)) {
                playLayer->removeFromParent();
                return false;
            }
        }
        
        // Aplicar input exacto
        if (c.player2 && playLayer->m_player2) {
            if (c.isHold) playLayer->m_player2->pushButton(PlayerButton::Jump);
            else playLayer->m_player2->releaseButton(PlayerButton::Jump);
        } else if (playLayer->m_player1) {
            if (c.isHold) playLayer->m_player1->pushButton(PlayerButton::Jump);
            else playLayer->m_player1->releaseButton(PlayerButton::Jump);
        }
    }
    
    // Avanzar hasta el frame objetivo con el offset aplicado
    while (currentFrame < targetFrame) {
        playLayer->update(1.0f / fps);
        currentFrame++;
        
        if (playLayer->m_player1->m_isDead || (playLayer->m_player2 && playLayer->m_player2->m_isDead)) {
            playLayer->removeFromParent();
            return false;
        }
    }
    
    // Si sobrevivió hasta aquí, el click es válido en este offset
    bool survived = !playLayer->m_player1->m_isDead && (!playLayer->m_player2 || !playLayer->m_player2->m_isDead);
    playLayer->removeFromParent();
    return survived;
}

int FrameAnalyzer::calculateFrameWindow(const ClickData& click, const std::vector<ClickData>& allClicks, const ScanConfig& config, float fps, const std::string& levelData) {
    int maxWindow = config.framesEachSide;
    
    // Buscar el máximo offset (hacia atrás y hacia adelante) donde el click sobrevive
    for (int offset = 0; offset <= maxWindow; ++offset) {
        bool surviveBefore = true;
        bool surviveAfter = true;
        
        if (offset > 0) {
            surviveBefore = simulateClickSurvival(click.frame, -offset, fps, allClicks, levelData);
            surviveAfter = simulateClickSurvival(click.frame, offset, fps, allClicks, levelData);
        }
        
        // Si falla en cualquier dirección, el frame window es el offset anterior
        if (!surviveBefore || !surviveAfter) {
            return offset - 1;
        }
    }
    
    return maxWindow; // Si sobrevive en todo el rango, es un click "fácil" (25+)
}

void FrameAnalyzer::analyzeAllClicks(std::vector<ClickData>& clicks, const ScanConfig& config, float levelDuration) {
    if (clicks.empty()) return;
    
    float fps = static_cast<float>(Mod::get()->getSettingValue<int64_t>("scan-fps"));
    float fromTime = (config.fromPercent / 100.0f) * levelDuration;
    float toTime = (config.toPercent / 100.0f) * levelDuration;
    
    for (size_t i = 0; i < clicks.size(); ++i) {
        auto& click = clicks[i];
        if (click.time < fromTime || click.time > toTime) {
            click.frameWindow = -1;
            continue;
        }
        click.internalClickIndex = static_cast<int>(i);
        click.frameWindow = calculateFrameWindow(click, clicks, config, fps, OlkFPC::get()->m_levelData);
    }
    
    detectSpecialClicks(clicks);
}

// ============================================================
//  DETECCIÓN DE STRAIGHT FLY Y WAVE SPAM
// ============================================================

void FrameAnalyzer::detectSpecialClicks(std::vector<ClickData>& clicks) {
    if (clicks.size() < 5) return;
    
    for (size_t i = 2; i < clicks.size() - 2; ++i) {
        int spacing1 = clicks[i].frame - clicks[i-1].frame;
        int spacing2 = clicks[i+1].frame - clicks[i].frame;
        int spacing3 = clicks[i+2].frame - clicks[i+1].frame;
        
        // Straight Fly: spacing constante de 2 a 4 frames
        if (spacing1 >= 2 && spacing1 <= 4 && spacing2 >= 2 && spacing2 <= 4 && spacing3 >= 2 && spacing3 <= 4) {
            if (std::abs(spacing1 - spacing2) <= 1 && std::abs(spacing2 - spacing3) <= 1) {
                clicks[i].isStraightFly = true;
            }
        }
        
        // Wave Spam: spacing de 1 frame exacto (cada frame)
        if (spacing1 == 1 && spacing2 == 1 && spacing3 == 1) {
            clicks[i].isWaveSpam = true;
        }
    }
}

// ============================================================
//  ESTADÍSTICAS Y DISTRIBUCIÓN
// ============================================================

std::vector<FrameWindowStats> FrameAnalyzer::computeDistribution(const std::vector<ClickData>& clicks, const std::vector<int>& customWindows) {
    std::map<int, int> counts;
    std::map<int, std::vector<int>> indices;
    
    for (size_t i = 0; i < clicks.size(); ++i) {
        int fw = clicks[i].frameWindow;
        if (fw < 0) continue;
        counts[fw]++;
        indices[fw].push_back(static_cast<int>(i));
    }
    
    int total = 0;
    for (auto& [w, c] : counts) total += c;
    
    std::vector<FrameWindowStats> result;
    for (int w : customWindows) {
        FrameWindowStats stat;
        stat.window = w;
        stat.count = counts.count(w) ? counts[w] : 0;
        stat.percentage = total > 0 ? (100.0f * stat.count / total) : 0.0f;
        stat.clickIndices = indices.count(w) ? indices[w] : std::vector<int>{};
        result.push_back(stat);
    }
    return result;
}

std::vector<HardestClick> FrameAnalyzer::findHardestClicks(const std::vector<ClickData>& clicks, int count) {
    std::vector<HardestClick> hardest;
    for (const auto& c : clicks) {
        if (c.frameWindow < 0) continue;
        HardestClick hc;
        hc.frameWindow = c.frameWindow;
        hc.clickIndex = c.internalClickIndex;
        hc.percentage = 100.0f / (c.frameWindow + 1);
        hc.label = "PRACTICE";
        hardest.push_back(hc);
    }
    
    std::sort(hardest.begin(), hardest.end(), [](const HardestClick& a, const HardestClick& b) {
        return a.frameWindow < b.frameWindow;
    });
    
    if (hardest.size() > static_cast<size_t>(count)) hardest.resize(count);
    return hardest;
}

InsightsData FrameAnalyzer::computeInsights(const std::vector<ClickData>& clicks, const std::vector<FrameWindowStats>& dist) {
    InsightsData data;
    data.totalClicks = static_cast<int>(clicks.size());
    
    std::vector<int> windows;
    for (const auto& c : clicks) {
        if (c.frameWindow >= 0) {
            windows.push_back(c.frameWindow);
            if (c.frameWindow == 0) data.exact++;
        }
    }
    
    if (windows.empty()) return data;
    std::sort(windows.begin(), windows.end());
    data.tightest = windows.front();
    data.median = windows[windows.size() / 2];
    data.busiestSecond = static_cast<int>(clicks.size() / 10);
    
    data.frameWindowDistribution = dist;
    data.hardestClicks = findHardestClicks(clicks, 10);
    return data;
}

FrameAnalyzer::ScanConfig FrameAnalyzer::getConfigFromSettings() {
    ScanConfig cfg;
    auto mod = Mod::get();
    std::string distStr = mod->getSettingValue<std::string>("scan-distance");
    cfg.framesEachSide = (distStr == "Auto") ? 25 : std::stoi(distStr);
    cfg.fromPercent = static_cast<float>(mod->getSettingValue<double>("scan-from"));
    cfg.toPercent = static_cast<float>(mod->getSettingValue<double>("scan-to"));
    cfg.speed = mod->getSettingValue<std::string>("scan-speed");
    cfg.localCheck = mod->getSettingValue<bool>("local-check");
    return cfg;
}