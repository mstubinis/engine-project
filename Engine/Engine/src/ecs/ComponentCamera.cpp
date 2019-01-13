#include "ecs/ComponentCamera.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/engine/Engine_ThreadManager.h"
#include "core/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


void epriv::ComponentCamera_Functions::RebuildProjectionMatrix(ComponentCamera& cam) {
    if (cam._type == ComponentCamera::Type::Perspective) {
        cam._projectionMatrix = glm::perspective(cam._angle, cam._aspectRatio, cam._nearPlane, cam._farPlane);
    }else{
        cam._projectionMatrix = glm::ortho(cam._left, cam._right, cam._bottom, cam._top, cam._nearPlane, cam._farPlane);
    }
}
glm::mat4 epriv::ComponentCamera_Functions::GetViewNoTranslation(Camera& c) {
    return c.m_Entity.getComponent<ComponentCamera>()->_viewMatrixNoTranslation;
}
glm::mat4 epriv::ComponentCamera_Functions::GetViewInverseNoTranslation(Camera& c) {
    return glm::inverse(c.m_Entity.getComponent<ComponentCamera>()->_viewMatrixNoTranslation);
}
glm::mat4 epriv::ComponentCamera_Functions::GetViewProjectionNoTranslation(Camera& c) {
    auto& component = *c.m_Entity.getComponent<ComponentCamera>();
    return component._projectionMatrix * component._viewMatrixNoTranslation;
}
glm::mat4 epriv::ComponentCamera_Functions::GetViewProjectionInverseNoTranslation(Camera& c) {
    auto& component = *c.m_Entity.getComponent<ComponentCamera>();
    return glm::inverse(component._projectionMatrix * component._viewMatrixNoTranslation);
}
glm::vec3 epriv::ComponentCamera_Functions::GetViewVectorNoTranslation(Camera& c) {
    auto& matrix = c.m_Entity.getComponent<ComponentCamera>()->_viewMatrixNoTranslation;
    return glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]);
}


#pragma region Component

ComponentCamera::ComponentCamera(Entity& _e,float angle, float aspectRatio, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::perspective(_angle, _aspectRatio, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Perspective;
}
ComponentCamera::ComponentCamera(Entity& _e,float left, float right, float bottom, float top, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::ortho(_left, _right, _bottom, _top, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Orthographic;
}
ComponentCamera::~ComponentCamera() {}
void ComponentCamera::resize(uint width, uint height) {
    if (_type == Type::Perspective) {
        _aspectRatio = width / (float)height;
    }
    epriv::ComponentCamera_Functions::RebuildProjectionMatrix(*this);
}
uint ComponentCamera::pointIntersectTest(glm::vec3& position) {
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > 0.0f) return 0; //outside
    }
    return 1;//inside
}
uint ComponentCamera::sphereIntersectTest(glm::vec3& position, float radius) {
    uint res = 1; //inside the viewing frustum
    if (radius <= 0.0f) return 0;
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > radius * 2.0f) return 0; //outside the viewing frustrum
        else if (d > 0.0f) res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentCamera::lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    _eye = eye;
    _up = up;
    _viewMatrix = glm::lookAt(_eye, center, _up);
    _viewMatrixNoTranslation = glm::lookAt(glm::vec3(0.0f), center - _eye, _up);
}
glm::mat4 ComponentCamera::getProjection() { return _projectionMatrix; }
glm::mat4 ComponentCamera::getProjectionInverse() { return glm::inverse(_projectionMatrix); }
glm::mat4 ComponentCamera::getView() { return _viewMatrix; }
glm::mat4 ComponentCamera::getViewInverse() { return glm::inverse(_viewMatrix); }
glm::mat4 ComponentCamera::getViewProjection() { return _projectionMatrix * _viewMatrix; }
glm::mat4 ComponentCamera::getViewProjectionInverse() { return glm::inverse(_projectionMatrix * _viewMatrix); }
glm::vec3 ComponentCamera::getViewVector() { return glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]); }
float ComponentCamera::getAngle() { return _angle; }
float ComponentCamera::getAspect() { return _aspectRatio; }
float ComponentCamera::getNear() { return _nearPlane; }
float ComponentCamera::getFar() { return _farPlane; }
void ComponentCamera::setAngle(float a) { _angle = a; epriv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); }
void ComponentCamera::setAspect(float a) { _aspectRatio = a; epriv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); }
void ComponentCamera::setNear(float n) { _nearPlane = n; epriv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); }
void ComponentCamera::setFar(float f) { _farPlane = f; epriv::ComponentCamera_Functions::RebuildProjectionMatrix(*this); }

#pragma endregion

#pragma region System

struct epriv::ComponentCamera_UpdateFunction final {
    static void _defaultUpdate(vector<uint>& _vec, vector<ComponentCamera>& _components,const float& dt) {
        for (uint j = 0; j < _vec.size(); ++j) {
            ComponentCamera& b = _components[_vec[j]];
            Math::extractViewFrustumPlanesHartmannGribbs(b._projectionMatrix * b._viewMatrix, b._planes);//update view frustrum 
        }
    }
    void operator()(void* _componentPool, const float& dt) const {
        auto& pool = *(ECSComponentPool<Entity, ComponentCamera>*)_componentPool;
        auto& components = pool.pool();

        auto split = epriv::threading::splitVectorIndices(components);
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components, dt);
        }
        epriv::threading::waitForAll();
    }
};
struct epriv::ComponentCamera_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentCamera_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentCamera_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentCamera_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentCamera_System::ComponentCamera_System() {
    setUpdateFunction(
        ComponentCamera_UpdateFunction());
    setOnComponentAddedToEntityFunction(
        ComponentCamera_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(
        ComponentCamera_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(
        ComponentCamera_SceneEnteredFunction());
    setOnSceneLeftFunction(
        ComponentCamera_SceneLeftFunction());
}

#pragma endregion