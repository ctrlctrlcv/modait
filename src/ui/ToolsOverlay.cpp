#include "ToolsOverlay.hpp"
#include <Geode/modify/PlayLayer.hpp>

// ============================================================
//  CREACIÓN
// ============================================================

ToolsOverlay* ToolsOverlay::create() {
    auto ret = new ToolsOverlay();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ToolsOverlay::init() {
    if (!CCNode::init()) return false;
    this->scheduleUpdate();
    return true;
}

void ToolsOverlay::clearNodes() {
    for (auto n : m_drawNodes) n->removeFromParent();
    for (auto l : m_labels) l->removeFromParent();
    m_drawNodes.clear();
    m_labels.clear();
}

// ============================================================
//  UPDATE PRINCIPAL
// ============================================================

void ToolsOverlay::update(float dt) {
    if (!m_visible) {
        this->setVisible(false);
        return;
    }
    this->setVisible(true);

    auto mod = Mod::get();
    clearNodes();

    // Dibujar cada herramienta según configuración
    if (mod->getSettingValue<bool>("show-hitboxes")) {
        drawHitboxes();
    }

    if (mod->getSettingValue<bool>("hitbox-trail")) {
        drawHitboxTrail();
    }

    if (mod->getSettingValue<bool>("show-trajectory")) {
        drawTrajectory();
    }

    if (mod->getSettingValue<bool>("frame-stepper")) {
        drawFrameStepper();
    }

    if (mod->getSettingValue<bool>("input-display")) {
        drawInputDisplay();
    }
}

// ============================================================
//  HITBOXES
// ============================================================

void ToolsOverlay::drawHitboxes() {
    auto playLayer = PlayLayer::get();
    if (!playLayer) return;

    auto drawNode = CCDrawNode::create();
    m_drawNodes.push_back(drawNode);
    this->addChild(drawNode);

    // Hitbox del jugador 1
    if (playLayer->m_player1) {
        auto pos = playLayer->m_player1->getPosition();
        auto size = playLayer->m_player1->getContentSize();

        // Hitbox del jugador (verde)
        drawNode->drawRect(
            {pos.x - size.width / 2, pos.y - size.height / 2},
            {pos.x + size.width / 2, pos.y + size.height / 2},
            {0, 255, 0, 180}
        );

        // Hitbox interna (roja, más pequeña)
        float innerW = size.width * 0.3f;
        float innerH = size.height * 0.3f;
        drawNode->drawRect(
            {pos.x - innerW / 2, pos.y - innerH / 2},
            {pos.x + innerW / 2, pos.y + innerH / 2},
            {255, 0, 0, 200}
        );
    }

    // Hitbox del jugador 2
    if (playLayer->m_player2) {
        auto pos = playLayer->m_player2->getPosition();
        auto size = playLayer->m_player2->getContentSize();

        drawNode->drawRect(
            {pos.x - size.width / 2, pos.y - size.height / 2},
            {pos.x + size.width / 2, pos.y + size.height / 2},
            {0, 200, 255, 180}
        );

        float innerW = size.width * 0.3f;
        float innerH = size.height * 0.3f;
        drawNode->drawRect(
            {pos.x - innerW / 2, pos.y - innerH / 2},
            {pos.x + innerW / 2, pos.y + innerH / 2},
            {255, 0, 0, 200}
        );
    }

    // Hitboxes on Death: si el jugador murió, dibujar en rojo brillante
    if (mod->getSettingValue<bool>("hitboxes-on-death")) {
        if (playLayer->m_player1 && playLayer->m_player1->m_isDead) {
            auto pos = playLayer->m_player1->getPosition();
            auto size = playLayer->m_player1->getContentSize();
            // Hitbox expandida en rojo
            drawNode->drawRect(
                {pos.x - size.width, pos.y - size.height},
                {pos.x + size.width, pos.y + size.height},
                {255, 0, 0, 255}
            );
        }
    }
}

// ============================================================
//  HITBOX TRAIL
// ============================================================

void ToolsOverlay::drawHitboxTrail() {
    auto playLayer = PlayLayer::get();
    if (!playLayer || !playLayer->m_player1) return;

    // Añadir punto actual al trail
    auto pos = playLayer->m_player1->getPosition();
    m_trail.push_back({pos, 0.0f});

    // Limitar el tamaño del trail
    if (m_trail.size() > 60) {
        m_trail.erase(m_trail.begin());
    }

    // Envejecer puntos
    for (auto& p : m_trail) {
        p.time += 1.0f / 60.0f;
    }

    // Dibujar trail
    auto drawNode = CCDrawNode::create();
    m_drawNodes.push_back(drawNode);
    this->addChild(drawNode);

    for (size_t i = 1; i < m_trail.size(); ++i) {
        float alpha = 1.0f - (static_cast<float>(i) / m_trail.size());
        alpha = std::clamp(alpha, 0.0f, 1.0f);

        cocos2d::ccColor4B color = {
            100, 255, 100,
            static_cast<uint8_t>(alpha * 180)
        };

        drawNode->drawSegment(
            m_trail[i - 1].pos,
            m_trail[i].pos,
            2.0f,
            color
        );
    }
}

// ============================================================
//  TRAYECTORIA
// ============================================================

void ToolsOverlay::drawTrajectory() {
    auto playLayer = PlayLayer::get();
    if (!playLayer || !playLayer->m_player1) return;

    auto drawNode = CCDrawNode::create();
    m_drawNodes.push_back(drawNode);
    this->addChild(drawNode);

    // Simular la trayectoria futura del jugador
    // (simplificado: línea recta hacia adelante)
    auto startPos = playLayer->m_player1->getPosition();
    auto velocity = playLayer->m_player1->m_yVelocity;

    cocos2d::CCPoint simPos = startPos;
    float simVelY = velocity;

    for (int i = 0; i < 30; ++i) {
        // Gravedad
        simVelY -= 1.0f;

        // Aplicar velocidad
        cocos2d::CCPoint nextPos = {
            simPos.x + 8.0f,  // velocidad X constante
            simPos.y + simVelY
        };

        // Dibujar segmento
        float alpha = 1.0f - (i / 30.0f);
        drawNode->drawSegment(
            simPos,
            nextPos,
            2.0f,
            {255, 255, 100, static_cast<uint8_t>(alpha * 200)}
        );

        simPos = nextPos;

        // Si toca el suelo, parar
        if (simPos.y < 0) break;
    }
}

// ============================================================
//  FRAME STEPPER
// ============================================================

void ToolsOverlay::drawFrameStepper() {
    auto playLayer = PlayLayer::get();
    if (!playLayer) return;

    // Mostrar información del frame actual
    int currentFrame = static_cast<int>(
        playLayer->m_gameState.m_levelTime * 240.0f);

    auto frameLabel = CCLabelBMFont::create(
        fmt::format("Frame: {}", currentFrame).c_str(),
        "bigFont.fnt");
    frameLabel->setScale(0.5f);
    frameLabel->setPosition(100, CCDirector::get()->getWinSize().height - 60);
    frameLabel->setColor({255, 255, 100});
    m_labels.push_back(frameLabel);
    this->addChild(frameLabel);

    // Instrucciones
    auto hintLabel = CCLabelBMFont::create(
        "Presiona [F] para step",
        "chatFont.fnt");
    hintLabel->setScale(0.4f);
    hintLabel->setPosition(100, CCDirector::get()->getWinSize().height - 80);
    hintLabel->setColor({150, 150, 180});
    m_labels.push_back(hintLabel);
    this->addChild(hintLabel);
}

// ============================================================
//  INPUT DISPLAY
// ============================================================

void ToolsOverlay::drawInputDisplay() {
    auto playLayer = PlayLayer::get();
    if (!playLayer || !playLayer->m_player1) return;

    // Mostrar inputs actuales
    bool isHolding = playLayer->m_player1->m_isHolding;
    bool isJumping = playLayer->m_player1->m_isGoingLeft ||
                     playLayer->m_player1->m_isGoingRight;

    // Panel de fondo
    auto panel = CCLayerColor::create({20, 20, 30, 200}, 200, 60);
    panel->setPosition(20, 100);
    m_drawNodes.push_back((CCDrawNode*)panel);
    this->addChild(panel);

    // Título
    auto title = CCLabelBMFont::create("INPUT DISPLAY", "chatFont.fnt");
    title->setScale(0.4f);
    title->setPosition(120, 145);
    title->setColor({180, 180, 220});
    m_labels.push_back(title);
    this->addChild(title);

    // Estado de hold
    auto holdLabel = CCLabelBMFont::create(
        isHolding ? "HOLD: ON" : "HOLD: OFF",
        "chatFont.fnt");
    holdLabel->setScale(0.45f);
    holdLabel->setPosition(120, 125);
    holdLabel->setColor(isHolding ? ccColor3B{100, 255, 100} : ccColor3B{100, 100, 120});
    m_labels.push_back(holdLabel);
    this->addChild(holdLabel);

    // Estado de movimiento
    std::string moveState = "IDLE";
    if (playLayer->m_player1->m_isGoingLeft) moveState = "LEFT";
    if (playLayer->m_player1->m_isGoingRight) moveState = "RIGHT";

    auto moveLabel = CCLabelBMFont::create(
        fmt::format("MOVE: {}", moveState).c_str(),
        "chatFont.fnt");
    moveLabel->setScale(0.45f);
    moveLabel->setPosition(120, 105);
    moveLabel->setColor({200, 200, 255});
    m_labels.push_back(moveLabel);
    this->addChild(moveLabel);
}