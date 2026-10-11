#include "FrameAnalyzer.hpp"
#include "OlkFPC.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

using namespace geode::prelude;

namespace fbfc {

// ===================== HeadlessSim =====================
namespace {
struct SimGuard {
    SimGuard() { OlkFPC::get().pushSim(); }
    ~SimGuard() { OlkFPC::get().popSim(); }
};
}

Result<> HeadlessSim::create(GJGameLevel* level, double fps) {
    destroy();
    if (!level) return Err("No level");
    if (level->m_levelString.empty()) return Err("Level data (levelString) is empty; download the level first");
    SimGuard g;
    auto* gm = GameManager::get();
    auto* real = gm->m_playLayer;                       // PlayLayer::init lo sobrescribe
    m_layer = PlayLayer::create(level, false, false);
    gm->m_playLayer = real;
    if (!m_layer) return Err("PlayLayer::create failed");
    m_layer->retain();
    m_dt = static_cast<float>(1.0 / std::max(1.0, fps));
    m_layer->resetLevel();
    return Ok();
}

void HeadlessSim::destroy() {
    if (!m_layer) return;
    SimGuard g;
    auto* gm = GameManager::get();
    auto* real = gm->m_playLayer;
    m_layer->release();
    gm->m_playLayer = real;
    m_layer = nullptr;
}

void HeadlessSim::input(bool down, bool p2) {
    if (!m_layer) return;
    SimGuard g;
    m_layer->handleButton(down, 1, !p2);
}
void HeadlessSim::step() { if (m_layer) { SimGuard g; m_layer->update(m_dt); } }
bool HeadlessSim::dead() const { return !m_layer || m_layer->m_player1->m_isDead; }
bool HeadlessSim::completed() const { return m_layer && m_layer->m_levelEndAnimationStarted; }
Ref<CheckpointObject> HeadlessSim::snapshot() { SimGuard g; return m_layer->createCheckpoint(); }
void HeadlessSim::restore(CheckpointObject* cp) { SimGuard g; m_layer->loadFromCheckpoint(cp); }

// ===================== FrameAnalyzer =====================
Result<> FrameAnalyzer::begin(GJGameLevel* level, MacroData const& macro) {
    cancel();
    if (macro.empty()) return Err("Macro is empty");
    auto* mod = Mod::get();
    m_scanFps = static_cast<double>(mod->getSettingValue<int64_t>("scan-fps"));
    m_maxShift = static_cast<int>(mod->getSettingValue<int64_t>("scan-max-shift"));
    m_horizon = static_cast<int>(mod->getSettingValue<int64_t>("scan-horizon"));
    m_macro = macro;
    m_ratio = m_scanFps / std::max(1.0, macro.fps);

    m_in.clear(); m_clicks.clear(); m_results.clear();
    for (size_t i = 0; i < macro.events.size(); ++i) {
        auto& e = macro.events[i];
        m_in.push_back({static_cast<int64_t>(std::llround(e.frame * m_ratio)), e.down, e.player2, i});
    }
    std::ranges::stable_sort(m_in, {}, &In::frame);
    for (size_t i = 0; i < m_in.size(); ++i) if (m_in[i].down) m_clicks.push_back(i);
    if (m_clicks.empty()) return Err("Macro has no clicks");

    GEODE_UNWRAP(m_sim.create(level, m_scanFps));
    m_phase = Phase::Advance;
    m_idx = 0; m_cursor = 0; m_baseFrame = 0;
    m_running = true;
    return Ok();
}

void FrameAnalyzer::cancel() { m_running = false; m_snap = nullptr; m_sim.destroy(); }

float FrameAnalyzer::progress() const {
    return m_clicks.empty() ? 0.f : static_cast<float>(m_idx) / static_cast<float>(m_clicks.size());
}

void FrameAnalyzer::applyAndStepBaseline() {
    while (m_cursor < m_in.size() && m_in[m_cursor].frame <= m_baseFrame) {
        m_sim.input(m_in[m_cursor].down, m_in[m_cursor].p2);
        ++m_cursor;
    }
    m_sim.step();
    ++m_baseFrame;
}

// Reproduce desde el snapshot con el click actual desplazado `shift` frames; true = sobrevive el horizonte.
bool FrameAnalyzer::runVariant(int shift) {
    m_sim.restore(m_snap);
    size_t cur = m_snapCursor;
    int64_t end = effFrame(m_curIn, shift) + m_horizon;
    for (int64_t f = m_snapFrame; f < end; ++f) {
        while (cur < m_in.size() && effFrame(cur, shift) <= f) {
            m_sim.input(m_in[cur].down, m_in[cur].p2);
            ++cur;
        }
        m_sim.step();
        if (m_sim.completed()) return true;
        if (m_sim.dead()) return false;
    }
    return true;
}

void FrameAnalyzer::finishAll() {
    // Los clicks restantes (baseline muerto) se marcan inválidos.
    for (; m_idx < m_clicks.size(); ++m_idx) {
        auto& in = m_in[m_clicks[m_idx]];
        ClickResult r; r.eventIndex = in.srcIndex; r.frame = m_macro.events[in.srcIndex].frame;
        r.timeMs = m_macro.events[in.srcIndex].timeMs; r.player2 = in.p2;
        r.posX = m_macro.events[in.srcIndex].posX; r.posY = m_macro.events[in.srcIndex].posY;
        m_results.push_back(r);
    }
    tagPatterns(m_results);
    m_running = false; m_snap = nullptr; m_sim.destroy();
}

bool FrameAnalyzer::step(double budgetMs) {
    if (!m_running) return true;
    auto t0 = std::chrono::steady_clock::now();
    auto elapsed = [&] { return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count(); };

    while (m_running && elapsed() < budgetMs) {
        if (m_idx >= m_clicks.size()) { finishAll(); break; }
        m_curIn = m_clicks[m_idx];
        int64_t f = m_in[m_curIn].frame;
        size_t prevIn = m_curIn > 0 ? m_curIn - 1 : SIZE_MAX;
        size_t nextIn = m_curIn + 1 < m_in.size() ? m_curIn + 1 : SIZE_MAX;

        switch (m_phase) {
        case Phase::Advance: {
            int64_t target = std::max<int64_t>(m_baseFrame, f - m_maxShift - 1);
            for (int i = 0; i < 256 && m_baseFrame < target; ++i) {
                applyAndStepBaseline();
                if (m_sim.dead()) { finishAll(); return true; }
            }
            if (m_baseFrame >= target) {
                m_snap = m_sim.snapshot();
                m_snapCursor = m_cursor; m_snapFrame = m_baseFrame;
                m_phase = Phase::Check0;
            }
        } break;
        case Phase::Check0:
            if (!runVariant(0)) {
                auto const& src = m_macro.events[m_in[m_curIn].srcIndex];
                ClickResult r; r.eventIndex = m_in[m_curIn].srcIndex; r.frame = src.frame; r.timeMs = src.timeMs;
                r.player2 = m_in[m_curIn].p2; r.posX = src.posX; r.posY = src.posY; r.window = -1;
                m_results.push_back(r);
                m_phase = Phase::Next;
            } else { m_phase = Phase::Back; m_shift = 1; m_back = 0; }
            break;
        case Phase::Back: {
            bool allowed = m_shift <= m_maxShift && f - m_shift >= m_snapFrame &&
                           (prevIn == SIZE_MAX || f - m_shift > m_in[prevIn].frame);
            if (allowed && runVariant(-m_shift)) { m_back = m_shift++; }
            else { m_phase = Phase::Fwd; m_shift = 1; m_fwd = 0; }
        } break;
        case Phase::Fwd: {
            bool allowed = m_shift <= m_maxShift && (nextIn == SIZE_MAX || f + m_shift < m_in[nextIn].frame);
            if (allowed && runVariant(m_shift)) { m_fwd = m_shift++; }
            else {
                auto const& src = m_macro.events[m_in[m_curIn].srcIndex];
                ClickResult r; r.eventIndex = m_in[m_curIn].srcIndex; r.frame = src.frame; r.timeMs = src.timeMs;
                r.player2 = m_in[m_curIn].p2; r.posX = src.posX; r.posY = src.posY;
                r.back = m_back; r.fwd = m_fwd; r.window = std::min(m_back, m_fwd);
                r.windowMs = r.window * 1000.0 / m_scanFps;
                m_results.push_back(r);
                m_phase = Phase::Next;
            }
        } break;
        case Phase::Next:
            m_sim.restore(m_snap);                 // vuelve al baseline
            m_cursor = m_snapCursor; m_baseFrame = m_snapFrame;
            ++m_idx; m_phase = Phase::Advance;
            break;
        }
    }
    return !m_running;
}

// ---------- Patrones: Straight Fly / Wave Spam (heurística por intervalos) ----------
void FrameAnalyzer::tagPatterns(std::vector<ClickResult>& r) {
    constexpr size_t kMinRun = 6;
    constexpr double kWaveMaxMs = 90.0, kFlyMaxMs = 260.0, kJitter = 0.20;
    auto classify = [](double ms) { return ms <= kWaveMaxMs ? PatternTag::WaveSpam : ms <= kFlyMaxMs ? PatternTag::StraightFly : PatternTag::None; };
    size_t i = 0;
    while (i + 1 < r.size()) {
        double base = r[i + 1].timeMs - r[i].timeMs;
        auto tag = classify(base);
        size_t j = i + 1;
        while (tag != PatternTag::None && j + 1 < r.size()) {
            double d = r[j + 1].timeMs - r[j].timeMs;
            if (classify(d) != tag || std::abs(d - base) > base * kJitter + 1.0) break;
            ++j;
        }
        if (tag != PatternTag::None && j - i + 1 >= kMinRun) for (size_t k = i; k <= j; ++k) r[k].pattern = tag;
        i = std::max(j, i + 1);
    }
}

Insights FrameAnalyzer::computeInsights(std::vector<ClickResult> const& all, double macroFps) {
    Insights s;
    std::vector<ClickResult> v;
    for (auto& r : all) if (r.valid()) v.push_back(r);
    s.total = v.size();
    if (v.empty()) return s;
    std::vector<int> w;
    for (auto& r : v) { w.push_back(r.window); ++s.buckets[bucketIndex(r.window)]; if (r.window == 0) ++s.exact; }
    std::ranges::sort(w);
    s.tightest = w.front();
    s.median = w.size() % 2 ? w[w.size() / 2] : (w[w.size() / 2 - 1] + w[w.size() / 2]) / 2.0;
    double span = static_cast<double>(v.back().frame - v.front().frame) / std::max(1.0, macroFps);
    s.clicksPerSec = span > 0 ? static_cast<double>(v.size()) / span : 0.0;
    std::ranges::stable_sort(v, [](auto& a, auto& b) { return a.window != b.window ? a.window < b.window : a.frame < b.frame; });
    s.hardest.assign(v.begin(), v.begin() + std::min<size_t>(10, v.size()));
    return s;
}

}  // namespace fbfc
