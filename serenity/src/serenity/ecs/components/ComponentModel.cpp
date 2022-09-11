#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentCamera.h>

#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/material/Material.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/system/Engine.h>
#include <serenity/scene/Scene.h>

using namespace Engine;
using namespace Engine::priv;

namespace {
    void internal_RegisterDeferredMeshLoaded(ComponentModel& modelComponent, Mesh* mesh) {
        if (mesh && !mesh->isLoaded()) {
            modelComponent.registerEvent(EventType::ResourceLoaded);
        }
    }
    size_t internal_GetTotalVertexCount(ComponentModel& modelComponent) {
        size_t totalCapacity = 0;
        for (const auto& modelInstance : modelComponent) {
            const Mesh& mesh = *modelInstance->getMesh().get<Mesh>();
            if (!mesh.isLoaded()) {
                continue;
            }
            totalCapacity += mesh.getVertexData().m_Data[0].getSize();
        }
        return totalCapacity;
    }
    std::pair<glm::vec3, float> internal_CalculateBoundingBoxAndRadius(const std::vector<glm::vec3>& points) {
        float     maxRadius = 0.0f;
        glm::vec3 maxBoundingBox{ 0.0f };
        for (const auto& point : points) {
            const auto abs_point = glm::abs(point);
            const float radius = glm::length(abs_point);
            maxRadius = std::max(maxRadius, radius);
            for (glm::length_t i = 0; i < maxBoundingBox.length(); ++i) {
                maxBoundingBox[i] = std::max(maxBoundingBox[i], abs_point[i]);
            }
        }
        return { maxBoundingBox, maxRadius };
    }
}

float ComponentModel_Functions::CalculateRadius(ComponentModel& modelComponent) {
    const uint32_t vertexCount = uint32_t(internal_GetTotalVertexCount(modelComponent));
    auto points_total          = Engine::create_and_reserve<std::vector<glm::vec3>>(vertexCount);
    const auto transform       = modelComponent.m_Owner.getComponent<ComponentTransform>();
    for (const auto& modelInstance : modelComponent) {
        const Mesh& mesh = *modelInstance->getMesh().get<Mesh>();
        if (!mesh.isLoaded()) {
            continue;
        }
        const float modelInstanceScale = Engine::Math::Max(modelInstance->getScale());
        glm::vec3 localPosition  = Engine::Math::getMatrixPosition(modelInstance->getModelMatrix());
        if (transform && transform->hasParent()) {
            localPosition += glm::vec3{ transform->getLocalPosition() };
        }
        const auto positions           = mesh.getVertexData().getPositions();
        for (const auto& vertexPosition : positions) {
            points_total.emplace_back(localPosition + (vertexPosition * modelInstanceScale));
        }
    }
    const auto boxAndRadius          = internal_CalculateBoundingBoxAndRadius(points_total);
    modelComponent.m_Radius          = boxAndRadius.second;
    modelComponent.m_RadiusBox       = boxAndRadius.first;
    if (transform) {
        const auto transformScale    = Engine::Math::Max(glm::vec3{ transform->getScale() });
        modelComponent.m_Radius     *= transformScale;
        modelComponent.m_RadiusBox  *= transformScale;
    }
    return modelComponent.m_Radius;
}
std::pair<glm::vec3, float> ComponentModel_Functions::CalculateRadi(const std::vector<Entity>& entities) {
    float maxRad = 0.0f;
    glm::vec3 maxRadBox{ 0.0f };
    for (const Entity entity : entities) {
        auto model     = entity.getComponent<ComponentModel>();
        const auto rad = ComponentModel_Functions::CalculateRadius(*model);
        maxRad         = std::max(maxRad, model->m_Radius);
        for (glm::length_t i = 0; i < maxRadBox.length(); ++i) {
            maxRadBox[i] = std::max(maxRadBox[i], model->m_RadiusBox[i]);
        }
    }
    return { maxRadBox, maxRad };
}
#pragma region Component

ComponentModel::ComponentModel(Entity entity, Handle mesh, Handle material, Handle shaderProgram, RenderStage stage)
    : m_Owner{ entity }
{
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(Entity entity, const std::string& meshName, const std::string& materialName, const std::string& shaderProgramName, RenderStage stage)
    : m_Owner{ entity }
{
    Handle mesh          = Engine::Resources::getResource<Mesh>(meshName);
    Handle material      = !materialName.empty() ? Engine::Resources::getResource<Material>(materialName) : Material::Checkers;
    Handle shaderProgram = !shaderProgramName.empty() ? Engine::Resources::getResource<ShaderProgram>(shaderProgramName) : ShaderProgram::Deferred;
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(ComponentModel&& other) noexcept 
    : m_ModelInstances { std::move(other.m_ModelInstances) }
    , m_RadiusBox      { std::move(other.m_RadiusBox) }
    , m_Radius         { std::move(other.m_Radius) }
    , m_Owner          { std::exchange(other.m_Owner, Entity{}) }
{
    if (other.isRegistered(EventType::ResourceLoaded)) {
        registerEvent(EventType::ResourceLoaded);
    }
}
ComponentModel& ComponentModel::operator=(ComponentModel&& other) noexcept {
    if (this != &other) {
        //TODO: cleanup model instances before the move occurs?
        m_ModelInstances = std::move(other.m_ModelInstances);
        m_RadiusBox      = std::move(other.m_RadiusBox);
        m_Radius         = std::move(other.m_Radius);
        m_Owner          = std::exchange(other.m_Owner, Entity{});
        if (other.isRegistered(EventType::ResourceLoaded)) {
            registerEvent(EventType::ResourceLoaded);
        }
    }
    return *this;
}
ComponentModel::~ComponentModel() {
    //TODO: cleanup model instances?
    unregisterEvent(EventType::ResourceLoaded);
}
void ComponentModel::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded && e.eventResource.resource->type() == ResourceType::Mesh) {
        bool finished = true;
        for (const auto& instance : m_ModelInstances) {
            if (!instance->getMesh().get<Mesh>()->isLoaded()) {
                finished = false;
                break;
            }
        }
        if (finished) {
            ComponentModel_Functions::CalculateRadius(*this);
            unregisterEvent(EventType::ResourceLoaded);
        }
    }
}
void ComponentModel::setViewportFlag(uint32_t flag) noexcept {
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->setViewportFlag(flag);
    }
}
void ComponentModel::addViewportFlag(uint32_t flag) noexcept {
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->addViewportFlag(flag);
    }
}
void ComponentModel::show(bool shown) noexcept { 
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->show(shown);
    }
}
ModelInstanceHandle ComponentModel::addModel(Handle mesh, Handle material, Handle shaderProgram, RenderStage renderStage) {
    internal_RegisterDeferredMeshLoaded(*this, mesh);

    auto& modelInstance        = *m_ModelInstances.emplace_back(std::make_unique<ModelInstance>(m_Owner, mesh, material, shaderProgram));
    modelInstance.m_Stage      = renderStage;
    modelInstance.m_MeshHandle = mesh;
    modelInstance.m_Index      = static_cast<uint32_t>(m_ModelInstances.size() - 1);

    PublicScene::AddModelInstanceToPipeline(*m_Owner.scene(), modelInstance, renderStage);
    ComponentModel_Functions::CalculateRadius(*this);
    return { modelInstance.m_Index, *this };
}
void ComponentModel::setModel(Handle mesh, Handle material, size_t index, Handle shaderProgram, RenderStage renderStage) {
    internal_RegisterDeferredMeshLoaded(*this, mesh);

    auto& model_instance                 = *m_ModelInstances[index];
    auto& scene                          = *m_Owner.scene();
    PublicScene::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.getStage());

    model_instance.m_ShaderProgramHandle = shaderProgram;
    model_instance.m_MeshHandle          = mesh;
    model_instance.m_MaterialHandle      = material;
    model_instance.m_Stage               = renderStage;
    model_instance.getRunningAnimations().setMesh(mesh);

    PublicScene::AddModelInstanceToPipeline(scene, model_instance, renderStage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle shaderProgram, size_t index, RenderStage renderStage) {
    auto& model_instance                 = *m_ModelInstances[index];
    auto& scene                          = *m_Owner.scene();
    PublicScene::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.getStage());

    model_instance.m_ShaderProgramHandle = shaderProgram;
    model_instance.m_Stage               = renderStage;

    PublicScene::AddModelInstanceToPipeline(scene, model_instance, renderStage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setStage(RenderStage renderStage, size_t index) {
    auto& model_instance   = *m_ModelInstances[index];
    auto& scene            = *m_Owner.scene();
    PublicScene::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.getStage());

    model_instance.m_Stage = renderStage;

    PublicScene::AddModelInstanceToPipeline(scene, model_instance, renderStage);
}
void ComponentModel::setModelMesh(Handle mesh, size_t index, RenderStage renderStage) {
    internal_RegisterDeferredMeshLoaded(*this, mesh);

    auto& model_instance        = *m_ModelInstances[index];
    auto& scene                 = *m_Owner.scene();

    PublicScene::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.getStage());

    model_instance.m_MeshHandle = mesh;
    model_instance.m_Stage      = renderStage;
    model_instance.getRunningAnimations().setMesh(mesh);

    PublicScene::AddModelInstanceToPipeline(scene, model_instance, renderStage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMaterial(Handle material, size_t index, RenderStage renderStage) {
    auto& model_instance            = *m_ModelInstances[index];
    auto& scene                     = *m_Owner.scene();
    PublicScene::RemoveModelInstanceFromPipeline(scene, model_instance, model_instance.getStage());

    model_instance.m_MaterialHandle = material;
    model_instance.m_Stage          = renderStage;

    PublicScene::AddModelInstanceToPipeline(scene, model_instance, renderStage);
}
bool ComponentModel::rayIntersectSphere(const ComponentTransform& cameraTransform) const noexcept {
    auto modelTransform = m_Owner.getComponent<ComponentTransform>();
    return Math::rayIntersectSphere(modelTransform->getPosition(), m_Radius, cameraTransform.getWorldPosition(), cameraTransform.getForward());
}
bool ComponentModel::rayIntersectSphere(const Camera& camera) const noexcept {
    return rayIntersectSphere(*camera.getComponent<ComponentTransform>());
}
void ComponentModel::setUserPointer(void* UserPointer) noexcept {
    for (auto& instance : m_ModelInstances) {
        instance->setUserPointer(UserPointer);
    }
}
void ComponentModel::setShadowCaster(bool isShadowCaster) noexcept {
    for (auto& modelInstance : m_ModelInstances) {
        modelInstance->setShadowCaster(isShadowCaster);
    }
}

#pragma endregion