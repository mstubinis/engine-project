#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include "core/engine/Engine_EventObject.h"

struct Handle;
class ShaderP;
class Mesh;
class Material;
class MeshInstance;

class ComponentModel;
class ComponentCamera;

namespace Engine {
    namespace epriv {
        struct ComponentModelUpdateFunction;
        struct ComponentModelEntityAddedToSceneFunction;
        struct ComponentModelComponentAddedToEntityFunction;
        struct ComponentModelSceneEnteredFunction;
        struct ComponentModelSceneLeftFunction;
        struct ComponentModelFunctions final {
            static float CalculateRadius(ComponentModel& super);
        };
    };
};

class ComponentModel : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentModelUpdateFunction;
    friend struct Engine::epriv::ComponentModelEntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentModelComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentModelSceneEnteredFunction;
    friend struct Engine::epriv::ComponentModelSceneLeftFunction;
    friend struct Engine::epriv::ComponentModelFunctions;
    friend class  ::ComponentCamera;
    private:
        std::vector<MeshInstance*> models;
        float _radius;
        glm::vec3 _radiusBox;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentModel(Entity&, Handle& meshHandle, Handle& materialHandle, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Mesh*, Handle& materialHandle, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Handle& meshHandle, Material*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Mesh*, Material*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        ComponentModel(Entity&, Handle& meshHandle, Handle& materialHandle, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Mesh*, Handle& materialHandle, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Handle& meshHandle, Material*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Entity&, Mesh*, Material*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel& other) = default;
        ComponentModel& operator=(const ComponentModel& other) = default;
        ComponentModel(ComponentModel&& other) noexcept = default;
        ComponentModel& operator=(ComponentModel&& other) noexcept = default;

        ~ComponentModel();

        uint getNumModels();
        float radius();
        glm::vec3 boundingBox();
        void show();
        void hide();

        MeshInstance& getModel(uint index = 0);

        uint addModel(Handle& meshHandle, Handle& materialHandle, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        uint addModel(Mesh*, Material*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModel(Handle& meshHandle, Handle& materialHandle, uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModel(Mesh*, Material*, uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle& meshHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMaterial(Material*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle& materialHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderP*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle& materialHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(ComponentCamera& camera);

        template<class T> void setCustomBindFunctor(const T& functor, uint index = 0) { models[index]->setCustomBindFunctor(functor); }
        template<class T> void setCustomUnbindFunctor(const T& functor, uint index = 0) { models[index]->setCustomUnbindFunctor(functor); }
};

class ComponentModelSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentModelSystem();
        ~ComponentModelSystem() = default;
};

#endif