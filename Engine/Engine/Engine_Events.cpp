#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"

using namespace Engine;
using namespace std;

epriv::EventManager* epriv::EventManager::m_EventManager = nullptr;

epriv::EventManager::EventManager(const char* name,uint w,uint h){
    m_Delta = 0;
    m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

    m_currentKey = m_previousKey = KeyboardKey::Unknown;
    m_currentButton = m_previousButton = MouseButton::Unknown;

    EventManager::m_EventManager = this;
}
epriv::EventManager::~EventManager(){ 
    m_KeyStatus.clear();
    m_MouseStatus.clear();
    EventManager::m_EventManager = nullptr;
}

void epriv::EventManager::setMousePositionInternal(float x, float y, bool resetDifference, bool resetPrevious) {
    glm::vec2 newPos = glm::vec2(x, y);
    resetPrevious ? m_Position_Previous = newPos : m_Position_Previous = m_Position;
    m_Position = newPos;
    m_Difference += (m_Position - m_Position_Previous);
    if (resetDifference) m_Difference = glm::vec2(0.0f);
}

void epriv::EventManager::onEventKeyPressed(uint& key){ 
    m_previousKey = m_currentKey;
    m_currentKey = key;
    m_KeyStatus[key] = true;
}
void epriv::EventManager::onEventKeyReleased(uint& key){ 
    m_previousKey = KeyboardKey::Unknown;
    m_currentKey = KeyboardKey::Unknown;
    m_KeyStatus[key] = false;
}
void epriv::EventManager::onEventMouseButtonPressed(uint& mouseButton){ 
    m_previousButton = m_currentButton;
    m_currentButton = mouseButton;
    m_MouseStatus[mouseButton] = true;
}
void epriv::EventManager::onEventMouseButtonReleased(uint& mouseButton){ 
    m_previousButton = MouseButton::Unknown;
    m_currentButton = MouseButton::Unknown;
    m_MouseStatus[mouseButton] = false;
}
void epriv::EventManager::onEventMouseWheelMoved(int& delta){ 
    m_Delta += (delta * 10);
}
void epriv::EventManager::onResetEvents(){ 
    m_previousKey = KeyboardKey::Unknown;
    m_currentKey = KeyboardKey::Unknown;
    for (auto iterator : m_KeyStatus) { iterator.second = false; }
    for (auto iterator : m_MouseStatus) { iterator.second = false; }
    float dt = (float)Resources::dt();
    m_Delta *= (1.0f - (glm::clamp(dt * 0.01f,0.0f,1.0f))) * 0.95f;

    m_Difference.x = 0.0f; m_Difference.y = 0.0f;
}
const bool Engine::isKeyDown(KeyboardKey::Key key){
    if (!epriv::EventManager::m_EventManager->m_KeyStatus[key]) return false; return true;
}
const bool Engine::isKeyDownOnce(KeyboardKey::Key key){
    bool res = Engine::isKeyDown(key);
    auto& mgr = *epriv::EventManager::m_EventManager;
    if (res && mgr.m_currentKey == key && (mgr.m_currentKey != mgr.m_previousKey)) return true; return false;
}
const bool Engine::isKeyUp(KeyboardKey::Key key){ return !Engine::isKeyDown(key); }
const bool Engine::isMouseButtonDown(MouseButton::Button button){
    if (!epriv::EventManager::m_EventManager->m_MouseStatus[button]) return false; return true;
}
const bool Engine::isMouseButtonDownOnce(MouseButton::Button button){
    bool res = Engine::isMouseButtonDown(button);
    auto& mgr = *epriv::EventManager::m_EventManager;
    if (res && mgr.m_currentButton == button && (mgr.m_currentButton != mgr.m_previousButton)) return true; return false;
}
const glm::vec2& Engine::getMouseDifference(){ 
    return epriv::EventManager::m_EventManager->m_Difference;
}
const glm::vec2& Engine::getMousePositionPrevious(){ 
    return epriv::EventManager::m_EventManager->m_Position_Previous;
}
const glm::vec2& Engine::getMousePosition(){ 
    return epriv::EventManager::m_EventManager->m_Position;
}
const float Engine::getMouseWheelDelta(){ 
    return epriv::EventManager::m_EventManager->m_Delta;
}
void Engine::setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(x),int(y)),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal(x,y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(uint x,uint y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x,y),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal((float)x,(float)y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::vec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(pos.x),int(pos.y)),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal(pos.x,pos.y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::uvec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x,pos.y),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal((float)pos.x,(float)pos.y,resetDifference,resetPreviousPosition);
}
