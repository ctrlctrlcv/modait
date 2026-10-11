#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "OlkFPC.hpp"
#include "PlaybackController.hpp"
#include "RenderManager.hpp"
#include "SoundManager.hpp"
#include "ui/CirclesOverlay.hpp"
#include "ui/MainMenuUI.hpp"
#include "ui/RowsOverlay.hpp"
#include "ui/ToolsOverlay.hpp"

using namespace geode::prelude;
using namespace fbfc;

$on_mod(Loaded) {
    log::info("FBFrameCounter loaded (open-source frame window analyzer)");
}

// ---------------- Captura de inputs ----------------
class $modify(FBPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton b) {
        bool r = PlayerObject::pushButton(b);
        OlkFPC::get().onPlayerInput(this, b, true);
        return r;
    }
    bool releaseButton(PlayerButton b) {
        bool r = PlayerObject::releaseButton(b);
        OlkFPC::get().onPlayerInput(this, b, false);
        return r;
    }
};

// ---------------- Ciclo de vida del nivel + overlays ----------------
class $modify(FBPlayLayer, PlayLayer) {
    struct Fields {
        CirclesOverlay* circles = nullptr;
        RowsOverlay* rows = nullptr;
        ToolsOverlay* tools = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        if (OlkFPC::get().isSimulating()) return true;      // PlayLayer headless: sin overlays
        OlkFPC::get().onLevelEnter(this);
        auto f = m_fields.self();
        f->circles = CirclesOverlay::attach(this);
        f->rows = RowsOverlay::attach(this);
        f->tools = ToolsOverlay::attach(this);
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (OlkFPC::get().isSimulating()) return;
        OlkFPC::get().onLevelReset(this);
    }

    void destroyPlayer(PlayerObject* p, GameObject* o) {
        PlayLayer::destroyPlayer(p, o);
        if (!OlkFPC::get().isSimulating()) OlkFPC::get().onDeath();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (!OlkFPC::get().isSimulating()) OlkFPC::get().onLevelComplete();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (OlkFPC::get().isSimulating()) return;
        auto f = m_fields.self();
        if (f->circles) f->circles->refresh();
        if (f->rows) f->rows->refresh();
        if (f->tools) f->tools->update(this);
        SoundManager::get().onFrame(OlkFPC::get().currentFrame());
    }

    void onQuit() {
        PlaybackController::get().stop();
        RenderManager::get().finish();
        OlkFPC::get().onLevelExit();
        PlayLayer::onQuit();
    }
};

// ---------------- Reproducción de macros ----------------
class $modify(FBGameLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer* self = this;
        auto* real = static_cast<GJBaseGameLayer*>(PlayLayer::get());
        if (!OlkFPC::get().isSimulating() && self == real) PlaybackController::get().inject(self);
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};
// ---------------- Render a vídeo: dt fijo + captura tras dibujar ----------------
class $modify(FBScheduler, CCScheduler) {
    void update(float dt) {
        if (RenderManager::get().isRendering()) dt = RenderManager::get().fixedDt();
        CCScheduler::update(dt);
        OlkFPC::get().tick();                                 // avanza el análisis por rebanadas de tiempo
    }
};

class $modify(FBDirector, CCDirector) {
    void drawScene() {
        CCDirector::drawScene();
        if (RenderManager::get().isRendering()) RenderManager::get().captureFrame();
    }
};

// ---------------- PauseLayer: acceso a la UI ----------------
class $modify(FBPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto win = CCDirector::get()->getWinSize();
        auto* menu = CCMenu::create();
        menu->setPosition({win.width - 50.f, 40.f});
        auto* open = CCMenuItemSpriteExtra::create(ButtonSprite::create("FB", 40, true, "goldFont.fnt", "GJ_button_01.png", 28.f, 0.7f),
                                                   this, menu_selector(FBPauseLayer::onFB));
        menu->addChild(open);
        if (Mod::get()->getSettingValue<bool>("tools-stepper")) {
            auto* step = CCMenuItemSpriteExtra::create(ButtonSprite::create(">|", 40, true, "goldFont.fnt", "GJ_button_04.png", 28.f, 0.7f),
                                                       this, menu_selector(FBPauseLayer::onStep));
            step->setPositionY(36.f);
            menu->addChild(step);
        }
        this->addChild(menu, 100);
    }
    void onFB(CCObject*) { MainMenuUI::open(); }
    void onStep(CCObject*) { ToolsOverlay::stepFrame(); }
};
