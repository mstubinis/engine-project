#include <core/engine/events/Observer.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/EventDispatcher.h>

using namespace Engine;

Observer::Observer() {

}
Observer::~Observer() {

}
void Observer::registerEvent(EventType::Type type) {
    priv::Core::m_Engine->m_EventModule.m_EventDispatcher.registerObject(*this, type);
}
void Observer::unregisterEvent(EventType::Type type) {
    priv::Core::m_Engine->m_EventModule.m_EventDispatcher.unregisterObject(*this, type);
}
bool Observer::isRegistered(EventType::Type type) const {
    return priv::Core::m_Engine->m_EventModule.m_EventDispatcher.isObjectRegistered(*this, type);
}

void Observer::onEvent(const Event& event) {

}