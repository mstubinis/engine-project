#include <core/engine/lights/RodLight.h>

#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

RodLight::RodLight(const glm_vec3& pos, const float rodLength, Scene* scene) : PointLight(LightType::Rod, pos, scene) {
    setRodLength(rodLength);

    auto* body = getComponent<ComponentBody>();
    if (body) { //evil, but needed. find out why...
        body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    }

    if (m_Type == LightType::Rod) {
        auto& rodLights = priv::InternalScenePublicInterface::GetRodLights(*scene);
        rodLights.push_back(this);
    }
}
RodLight::~RodLight() {
}
float RodLight::calculateCullingRadius() {
    float res = PointLight::calculateCullingRadius();
    auto& body = *getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    return res;
}
void RodLight::setRodLength(const float length) {
    m_RodLength = length;
    auto& body = *getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
const float RodLight::rodLength() const {
    return m_RodLength; 
}
void RodLight::destroy() {
    EntityWrapper::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetRodLights(m_Entity.scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(m_Entity.scene()), this);
}