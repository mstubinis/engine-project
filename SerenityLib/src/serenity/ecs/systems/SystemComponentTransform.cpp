#include <serenity/ecs/systems/SystemComponentTransform.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/math/Engine_Math.h>

SystemComponentTransform::SystemComponentTransform(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{

    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = (SystemComponentTransform&)inSystem;
        auto& systemPC = inSystem.getECS().getSystem<SystemTransformParentChild>();
        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArgs, Entity entity, ComponentTransform* transform) {
            auto& worldMatrix = pcsArgs->m_WorldTransforms[entity.id() - 1];
            auto& localMatrix = pcsArgs->m_LocalTransforms[entity.id() - 1];
            Engine::Math::setFinalModelMatrix(localMatrix, transform->m_Position, transform->m_Rotation, transform->m_Scale);
            worldMatrix = localMatrix;
        }, &systemPC, SystemExecutionPolicy::ParallelWait);

        /*
        system.forEach<float>([](float dt, Entity entity, ComponentTransform* transform) {
            transform->m_Position += (transform->m_LinearVelocity * (decimal)dt);
        }, dt, SystemExecutionPolicy::ParallelWait);
        */
    });

    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemComponentTransform&)inSystem;
        auto model    = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
}