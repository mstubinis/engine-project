#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

float ComponentModel_Functions::CalculateRadius(ComponentModel& modelComponent) {
    vector<glm::vec3> points_total; //TODO: vector.reserve for performance here?
    for (size_t i = 0; i < modelComponent.m_ModelInstances.size(); ++i) {
        auto& modelInstance            = *modelComponent.m_ModelInstances[i];
        auto& mesh                     = *modelInstance.mesh();
        const auto  modelInstanceScale = Math::Max(modelInstance.getScale());
        const glm::vec3  localPosition = Math::getMatrixPosition(modelInstance.modelMatrix());
        auto data = const_cast<VertexData&>(mesh.getVertexData()).getData<glm::vec3>(0);
        for (size_t j = 0; j < data.size(); ++j) {
            points_total.push_back(localPosition + (data[j] * modelInstanceScale));
        }
    }
    float maxRadius      = 0.0f;
    auto  maxBoundingBox = glm::vec3(0.0f);
    for (auto& point : points_total) {
        const auto abs_point = glm::abs(point);
        const float radius   = glm::length(abs_point);
        if (radius > maxRadius)
            maxRadius = radius;
        if (abs_point.x > maxBoundingBox.x)
            maxBoundingBox.x = abs_point.x;
        if (abs_point.y > maxBoundingBox.y)
            maxBoundingBox.y = abs_point.y;
        if (abs_point.z > maxBoundingBox.z)
            maxBoundingBox.z = abs_point.z;
    }
    modelComponent.m_Radius    = maxRadius;
    modelComponent.m_RadiusBox = maxBoundingBox;
    auto* body                 = modelComponent.m_Owner.getComponent<ComponentBody>();
    if (body) {
        const auto bodyScale =  Math::Max(glm::vec3(body->getScale()));
        modelComponent.m_Radius       *= bodyScale;
        modelComponent.m_RadiusBox    *= bodyScale;
    }
    return modelComponent.m_Radius; //now modified by the body scale
};


#pragma region Component

ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity){
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material,  ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material,  ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(m_ModelInstances);
}

void ComponentModel::setViewportFlag(const unsigned int flag) {
    for (auto& model_instance : m_ModelInstances) {
        if(model_instance) model_instance->setViewportFlag(flag);
    }
}
void ComponentModel::addViewportFlag(const unsigned int flag) {
    for (auto& model_instance : m_ModelInstances) {
        if (model_instance) model_instance->addViewportFlag(flag);
    }
}
void ComponentModel::setViewportFlag(const ViewportFlag::Flag flag) {
    for (auto& model_instance : m_ModelInstances) {
        if (model_instance) model_instance->setViewportFlag(flag);
    }
}
void ComponentModel::addViewportFlag(const ViewportFlag::Flag flag) {
    for (auto& model_instance : m_ModelInstances) {
        if (model_instance) model_instance->addViewportFlag(flag);
    }
}

const size_t ComponentModel::getNumModels() const {
    return m_ModelInstances.size();
}
ModelInstance& ComponentModel::getModel(const size_t& index) {
    return *m_ModelInstances[index];
}
void ComponentModel::show() { 
    for (auto& modelInstance : m_ModelInstances)
        modelInstance->show();
}
void ComponentModel::hide() { 
    for (auto& modelInstance : m_ModelInstances)
        modelInstance->hide();
}
const float& ComponentModel::radius() const {
    return m_Radius; 
}
const glm::vec3& ComponentModel::boundingBox() const {
    return m_RadiusBox;
}
ModelInstance& ComponentModel::addModel(Handle& mesh, Handle& material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)material.get(), shaderProgram, stage);
}
ModelInstance& ComponentModel::addModel(Mesh* mesh, Material* material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    auto modelInstance = NEW ModelInstance(m_Owner, mesh, material, shaderProgram);
    const auto index = m_ModelInstances.size();
    modelInstance->m_Index = index;
    m_ModelInstances.push_back(modelInstance);
    auto& instance = *modelInstance;
    auto& _scene = m_Owner.scene();
    instance.m_Stage = stage;
    InternalScenePublicInterface::AddModelInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
    return instance;
}

ModelInstance& ComponentModel::addModel(Handle& mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) {
    return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)material.get(), (ShaderProgram*)shaderProgram.get(), stage);
}
ModelInstance& ComponentModel::addModel(Mesh* mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) {
    return ComponentModel::addModel(mesh, material, (ShaderProgram*)shaderProgram.get(), stage);
}


void ComponentModel::setModel(Handle& mesh, Handle& material, const size_t& index, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    ComponentModel::setModel((Mesh*)mesh.get(), (Material*)material.get(), index, shaderProgram, stage);
}
void ComponentModel::setModel(Mesh* mesh, Material* material, const size_t& index, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    auto& instance = *m_ModelInstances[index];
    auto& _scene = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Mesh          = mesh;
    instance.m_Material      = material;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderProgram* shaderProgram, const size_t& index, const RenderStage::Stage& stage) {
    auto& instance = *m_ModelInstances[index];
    auto& scene   = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, const size_t& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelShaderProgram((ShaderProgram*)shaderPHandle.get(), index, stage); 
}
void ComponentModel::setStage(const RenderStage::Stage& stage, const size_t& index) {
    auto& instance = *m_ModelInstances[index];
    auto& scene = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
}
void ComponentModel::setModelMesh(Mesh* mesh, const size_t& index, const RenderStage::Stage& stage) {
    auto& instance = *m_ModelInstances[index];
    auto& scene   = m_Owner.scene();

    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Mesh  = mesh;
    instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle& mesh, const size_t& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelMesh((Mesh*)mesh.get(), index, stage); 
}
void ComponentModel::setModelMaterial(Material* material, const size_t& index, const RenderStage::Stage& stage) {
    auto& instance = *m_ModelInstances[index];
    auto& scene   = m_Owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Material = material;
    instance.m_Stage    = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
}
void ComponentModel::setModelMaterial(Handle& material, const size_t& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelMaterial((Material*)(material.get()), index, stage);
}
const bool ComponentModel::rayIntersectSphere(const ComponentCamera& camera) {
    auto& body = *m_Owner.getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body.position(), m_Radius, camera.m_Eye, camera.getViewVector());
}
void ComponentModel::setUserPointer(void* UserPointer) {
    for (auto& instance : m_ModelInstances) {
        instance->setUserPointer(UserPointer);
    }
}


#pragma endregion

#pragma region System

struct epriv::ComponentModel_UpdateFunction final { void operator()(void* componentPool, const double& dt, Scene& scene) const {

}};
struct epriv::ComponentModel_ComponentAddedToEntityFunction final {void operator()(void* component, Entity& entity) const {

}};
struct epriv::ComponentModel_EntityAddedToSceneFunction final {void operator()(void* componentPool, Entity& entity, Scene& scene) const { 

}};
struct epriv::ComponentModel_SceneEnteredFunction final {void operator()(void* componentPool, Scene& scene) const {

}};
struct epriv::ComponentModel_SceneLeftFunction final {void operator()(void* componentPool, Scene& scene) const {

}};


ComponentModel_System_CI::ComponentModel_System_CI() {
    setUpdateFunction(ComponentModel_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentModel_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentModel_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentModel_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentModel_SceneLeftFunction());
}

#pragma endregion