#include <core/engine/sounds/SoundBaseClass.h>

SoundBaseClass::SoundBaseClass(const uint& numLoops) {
    m_Status      = SoundStatus::Fresh;
    m_Loops       = numLoops;
    m_CurrentLoop = 0;
}
SoundBaseClass::~SoundBaseClass() {
    m_Status = SoundStatus::Stopped;
}
const SoundStatus::Status& SoundBaseClass::status() {
    return m_Status;
}
void SoundBaseClass::update(const double& dt) {
}
const bool SoundBaseClass::play(const uint& numLoops) {
    return false;
}
const bool SoundBaseClass::pause() {
    return false;
}
const bool SoundBaseClass::stop() {
    return false;
}
const bool SoundBaseClass::restart() {
    return false;
}
const uint& SoundBaseClass::getLoopsLeft() {
    return m_Loops - m_CurrentLoop;
}
const float SoundBaseClass::getAttenuation() {
    return 0;
}
const glm::vec3& SoundBaseClass::getPosition() {
    return glm::vec3(0.0f);
}
void SoundBaseClass::setPosition(const float& x, const float& y, const float& z) {
}
void SoundBaseClass::setPosition(const glm::vec3& position) {
}
const float SoundBaseClass::getVolume() {
    return 0;
}
void SoundBaseClass::setVolume(const float& volume) {
}
const float SoundBaseClass::getPitch() {
    return 0;
}
void SoundBaseClass::setPitch(const float& pitch) {
}