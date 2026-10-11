#pragma once
#include <Geode/Geode.hpp>
#include <cmath>

namespace fbfc {

// Slider ligado a un setting del mod; admite escala lineal o logarítmica (1..30000 FPS).
class SettingSlider : public cocos2d::CCNode {
public:
    static SettingSlider* create(std::string key, std::string label, double mn, double mx, bool log, bool isInt) {
        auto* r = new SettingSlider();
        if (r->init(std::move(key), std::move(label), mn, mx, log, isInt)) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    void syncFromSetting() {
        auto v = read();
        m_slider->setValue(norm(v));
        updateLabel(v);
    }

private:
    bool init(std::string key, std::string label, double mn, double mx, bool log, bool isInt) {
        if (!CCNode::init()) return false;
        m_key = std::move(key); m_name = std::move(label); m_min = mn; m_max = mx; m_log = log; m_int = isInt;
        this->setContentSize({190.f, 24.f});
        m_label = cocos2d::CCLabelBMFont::create("", "bigFont.fnt");
        m_label->setScale(0.32f);
        m_label->setAnchorPoint({0.f, 0.5f});
        m_label->setPosition({0.f, 20.f});
        this->addChild(m_label);
        m_slider = Slider::create(this, menu_selector(SettingSlider::onSlide), 0.55f);
        m_slider->setPosition({95.f, 5.f});
        this->addChild(m_slider);
        syncFromSetting();
        return true;
    }

    double read() const {
        auto* mod = geode::Mod::get();
        return m_int ? static_cast<double>(mod->getSettingValue<int64_t>(m_key)) : mod->getSettingValue<double>(m_key);
    }
    double fromNorm(float t) const {
        double v = m_log ? std::exp(std::log(m_min) + t * (std::log(m_max) - std::log(m_min))) : m_min + t * (m_max - m_min);
        return m_int ? std::round(v) : v;
    }
    float norm(double v) const {
        v = std::clamp(v, m_min, m_max);
        return static_cast<float>(m_log ? (std::log(v) - std::log(m_min)) / (std::log(m_max) - std::log(m_min))
                                        : (v - m_min) / (m_max - m_min));
    }
    void updateLabel(double v) {
        m_label->setString(m_int ? fmt::format("{}: {}", m_name, static_cast<int64_t>(v)).c_str()
                                 : fmt::format("{}: {:.2f}", m_name, v).c_str());
    }
    void onSlide(CCObject* sender) {
        double v = fromNorm(static_cast<SliderThumb*>(sender)->getValue());
        auto* mod = geode::Mod::get();
        if (m_int) (void)mod->setSettingValue<int64_t>(m_key, static_cast<int64_t>(v));
        else (void)mod->setSettingValue<double>(m_key, v);
        updateLabel(v);
    }

    std::string m_key, m_name;
    double m_min = 0, m_max = 1;
    bool m_log = false, m_int = false;
    Slider* m_slider = nullptr;
    cocos2d::CCLabelBMFont* m_label = nullptr;
};

}  // namespace fbfc
