#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "OlkFPC.hpp"
#include "ui/MainMenuUI.hpp"
#include "ui/CirclesOverlay.hpp"
#include "PlaybackController.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("OlkFPC: Mod cargado correctamente");
    OlkFPC::get()->onModLoaded();
}

class $modify(OlkFPCPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this, menu_selector(OlkFPCPauseLayer::onOlkFPCButton));
        btn->setScale(0.8f);

        auto menu = this->getChildByID("left-button-menu");
        if (!menu) {
            menu = CCMenu::create();
            menu->setID("olkfpc-menu");
            this->addChild(menu);
        }
        menu->addChild(btn);
        btn->setPosition(0, 0);
    }

    void onOlkFPCButton(CCObject*) {
        MainMenuUI::create()->show();
    }
};

class $modify(OlkFPCPlayLayer, PlayLayer) {
    struct Fields {
        CirclesOverlay* circlesOverlay = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        m_fields->circlesOverlay = CirclesOverlay::create();
        this->addChild(m_fields->circlesOverlay, 1000);

        OlkFPC::get()->onLevelStart();
        OlkFPC::get()->m_levelName = level->m_levelName;
        OlkFPC::get()->m_levelID = std::to_string(level->m_levelID);
        OlkFPC::get()->m_levelData = level->m_levelString;

        return true;
    }

    void onQuit() {
        OlkFPC::get()->onLevelQuit();
        PlayLayer::onQuit();
    }
};

class $modify(OlkFPCPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);

        auto& olk = *OlkFPC::get();
        if (olk.m_isRecording) {
            ClickData click;
            auto playLayer = PlayLayer::get();
            if (playLayer) {
                click.frame = static_cast<int>(playLayer->m_gameState.m_levelTime * 240.0f);
                click.isHold = true;
                click.player2 = (this == playLayer->m_player2);
                olk.m_clicks.push_back(click);
            }
        }
    }
};
