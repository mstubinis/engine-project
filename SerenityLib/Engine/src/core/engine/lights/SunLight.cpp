#include <core/engine/lights/SunLight.h>

using namespace Engine;
using namespace std;


SunLight::SunLight(glm::vec3 pos, LightType::Type type, Scene* scene) :EntityWrapper(*scene){
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    m_Active = true;
    m_Color = glm::vec4(1.0f);
    m_Type = type;
    m_AmbientIntensity = 0.005f;
    m_DiffuseIntensity = 2.0f;
    m_SpecularIntensity = 1.0f;

    if(type == LightType::Sun)
        epriv::InternalScenePublicInterface::GetSunLights(*scene).push_back(this);

    auto& body = *m_Entity.addComponent<ComponentBody>();
    body.setPosition(pos);
}
SunLight::~SunLight() {
}
glm::vec3 SunLight::position() { return m_Entity.getComponent<ComponentBody>()->position(); }
void SunLight::setColor(float r, float g, float b, float a) { m_Color = glm::vec4(r, g, b, a); }
void SunLight::setColor(glm::vec4 col) { m_Color = col; }
void SunLight::setColor(glm::vec3 col) { m_Color.r = col.r; m_Color.g = col.g; m_Color.b = col.b; }
void SunLight::setPosition(float x, float y, float z) { m_Entity.getComponent<ComponentBody>()->setPosition(x, y, z); }
void SunLight::setPosition(glm::vec3 pos) { m_Entity.getComponent<ComponentBody>()->setPosition(pos); }
float SunLight::getAmbientIntensity() { return m_AmbientIntensity; }
void SunLight::setAmbientIntensity(float a) { m_AmbientIntensity = a; }
float SunLight::getDiffuseIntensity() { return m_DiffuseIntensity; }
void SunLight::setDiffuseIntensity(float d) { m_DiffuseIntensity = d; }
float SunLight::getSpecularIntensity() { return m_SpecularIntensity; }
void SunLight::setSpecularIntensity(float s) { m_SpecularIntensity = s; }
void SunLight::activate(bool b) { m_Active = b; }
void SunLight::deactivate() { m_Active = false; }
bool SunLight::isActive() { return m_Active; }
uint SunLight::type() { return m_Type; }