#include "HUD.h"

#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"
#include "../Planet.h"

#include "../hud/SensorStatusDisplay.h"
#include "../hud/ShipStatusDisplay.h"
#include "../hud/TargetRedicule.h"

#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>

using namespace std;
using namespace Engine;


HUD::HUD(Map& map, Font& font):m_Map(map), m_Font(font){
    m_Shown = true;
    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto textureWidth = radarTexture.width();

    Texture& background = *(Texture*)((ResourceManifest::ShipStatusBackgroundHUDTexture).get());

    m_SensorDisplay = new SensorStatusDisplay(*this,map,glm::vec2(winSize.x / 2.0f, 0),glm::vec2(textureWidth, radarTexture.height()),glm::vec4(1, 1, 0, 1),Alignment::BottomCenter);
    m_ShipStatusDisplay = new ShipStatusDisplay(*this, glm::vec2((winSize.x / 2.0f) - ((m_SensorDisplay->size().x / 2.0f) + 30), 0), background.size(), glm::vec4(1,1,0,1), Alignment::BottomRight);
    m_ShipTargetStatusDisplay = new ShipStatusDisplay(*this, glm::vec2((winSize.x / 2.0f) + ((m_SensorDisplay->size().x / 2.0f) + 30), 0), background.size(), glm::vec4(1, 1, 0, 1), Alignment::BottomLeft);
    m_TargetRedicule = new TargetRedicule(map, font);
}
HUD::~HUD() {
    SAFE_DELETE(m_SensorDisplay);
    SAFE_DELETE(m_ShipStatusDisplay);
    SAFE_DELETE(m_ShipTargetStatusDisplay);
    SAFE_DELETE(m_TargetRedicule);
}
const bool HUD::isHidden() const {
    return !m_Shown;
}
void HUD::show(const bool show_) {
    m_Shown = show_;
}
void HUD::hide() {
    m_Shown = false;
}
void HUD::toggle() {
    m_Shown = !m_Shown;
}
Font& HUD::getFont() {
    return m_Font;
}
SensorStatusDisplay& HUD::getSensorDisplay() {
    return *m_SensorDisplay;
}
ShipStatusDisplay& HUD::getShipStatusDisplay() {
    return *m_ShipStatusDisplay;
}
ShipStatusDisplay& HUD::getShipTargetStatusDisplay() {
    return *m_ShipTargetStatusDisplay;
}
void HUD::onResize(const unsigned int& width, const unsigned int& height) {
    m_SensorDisplay->onResize(width, height);
    m_ShipStatusDisplay->setPosition((width / 2.0f) - ((m_SensorDisplay->size().x / 2.0f) + 30), 0);
    m_ShipTargetStatusDisplay->setPosition((width / 2.0f) + ((m_SensorDisplay->size().x / 2.0f) + 30), 0);
}
void HUD::setTarget(Planet* planet) {
    m_ShipTargetStatusDisplay->setTarget(planet);
}
void HUD::setTarget(Ship* ship) {
    m_ShipTargetStatusDisplay->setTarget(ship);
}
void HUD::setTarget(EntityWrapper* wrapper) {
    m_ShipTargetStatusDisplay->setTarget(wrapper);
}
void HUD::setTarget(Entity& entity) {
    m_ShipTargetStatusDisplay->setTarget(entity);
}
uint _countShips = 0;
uint _countPlanets = 0;
void HUD::update(const double& dt) {
    if (Engine::isKeyDownOnce(KeyboardKey::LeftAlt, KeyboardKey::X) || Engine::isKeyDownOnce(KeyboardKey::RightAlt, KeyboardKey::X)) {
        toggle();
    }
    auto& player = *m_Map.getPlayer();
    auto& playerKey = player.getMapKey();

    if (Engine::isKeyDownOnce(KeyboardKey::Comma)) {
        const auto& ships = m_Map.getShipsPlayerControlled();
        vector<Ship*> shipsVect;
        shipsVect.reserve(ships.size());

        for (auto& p : ships) {
            if (p.second->getMapKey() != playerKey) {
                shipsVect.push_back(p.second);
            }
        }
        if (shipsVect.size() > 0) {
            if (_countShips > shipsVect.size() - 1) {
                _countShips = 0;
            }
            player.setTarget(shipsVect[_countShips]->getMapKey(), true);
            ++_countShips;
        }
    }else if (Engine::isKeyDownOnce(KeyboardKey::Period)) {
        const auto& planets = m_Map.getPlanets();
        vector<Planet*> planetsVect;
        planetsVect.reserve(planets.size());

        for (auto& p : planets) {
            planetsVect.push_back(p.second);
        }

        if (_countPlanets > planetsVect.size() - 1) {
            _countPlanets = 0;
        }
        player.setTarget(planetsVect[_countPlanets]->entity().getComponent<ComponentName>()->name(), true);
        ++_countPlanets;
    }

    m_SensorDisplay->update(dt);
    m_ShipStatusDisplay->update(dt);
    m_ShipTargetStatusDisplay->update(dt);
    m_TargetRedicule->update(dt);
}
void HUD::render() {
    if (!m_Shown) 
        return;

    m_SensorDisplay->render();
    m_ShipStatusDisplay->render();
    m_ShipTargetStatusDisplay->render();
    m_TargetRedicule->render();

#pragma region DrawDebugStuff
    m_Font.renderText(Engine::Data::reportTime() +
        epriv::Core::m_Engine->m_DebugManager.reportDebug(),
        glm::vec2(10.0f, Resources::getWindowSize().y - 10.0f), glm::vec4(1,1,0, 1.0f), 0, glm::vec2(0.8f, 0.8f), 0.1f);

#pragma endregion
}