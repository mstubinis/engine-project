#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include "ecs/ECS.h"

#include <iostream>

struct Entity;
class Scene;
namespace Engine {
    namespace epriv {

        struct FunctorUpdateEmpty final { void operator()(void* _cPool, const float& dt) const { } };
        struct FunctorComponentEmpty final { void operator()(void* _component) const { } };
        struct FunctorEntityEmpty final { void operator()(void* _cPool, Entity& _entity) const { } };
        struct FunctorSceneEmpty final { void operator()(void* _cPool, Scene& _Scene) const { } };

        template<typename ...> class ECSSystem;

        typedef boost::function<void(void*, const float&)>  func_update;
        typedef boost::function<void(void*, Entity&)>       func_entity;
        typedef boost::function<void(void*)>                func_component;
        typedef boost::function<void(void*,Scene&)>         func_scene;

        template<typename T> struct FunctorHolder {
            T functor;
            FunctorHolder() = default;
            FunctorHolder(const T& _f) { functor = _f; }
            virtual ~FunctorHolder() = default;
        };
        struct FunctorUpdate final : public FunctorHolder<func_update>{
            FunctorUpdate() { FunctorHolder<func_update>::functor = FunctorUpdateEmpty(); }
            FunctorUpdate(const func_update& _f) { FunctorHolder<func_update>::functor = _f; }
        };
        struct FunctorComponent : public FunctorHolder<func_component> {
            FunctorComponent() { FunctorHolder<func_component>::functor = FunctorComponentEmpty(); }
            FunctorComponent(const func_component& _f) { FunctorHolder<func_component>::functor = _f; }
        };
        struct FunctorEntity final : public FunctorHolder<func_entity> {
            FunctorEntity() { FunctorHolder<func_entity>::functor = FunctorEntityEmpty(); }
            FunctorEntity(const func_entity& _f) { FunctorHolder<func_entity>::functor = _f; }
        };
        struct FunctorScene final : public FunctorHolder<func_scene> {
            FunctorScene() { FunctorHolder<func_scene>::functor = FunctorSceneEmpty(); }
            FunctorScene(const func_scene& _f) { FunctorHolder<func_scene>::functor = _f; }
        };

        struct ECSSystemCI {
            FunctorUpdate        updateFunction;
            FunctorComponent     onComponentAddedToEntityFunction;
            FunctorEntity        onEntityAddedToSceneFunction;
            FunctorScene         onSceneEnteredFunction;
            FunctorScene         onSceneLeftFunction;

            ECSSystemCI() = default;
            virtual ~ECSSystemCI() = default;
            ECSSystemCI(const ECSSystemCI&) = delete;
            ECSSystemCI& operator=(const ECSSystemCI&) = delete;
            ECSSystemCI(ECSSystemCI&& other) noexcept = delete;
            ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete;

            template<typename T> void setUpdateFunction(const T& functor) { updateFunction = FunctorUpdate(functor); }
            template<typename T> void setOnComponentAddedToEntityFunction(const T& functor) { onComponentAddedToEntityFunction = FunctorComponent(functor); }
            template<typename T> void setOnEntityAddedToSceneFunction(const T& functor) { onEntityAddedToSceneFunction = FunctorEntity(functor); }
            template<typename T> void setOnSceneEnteredFunction(const T& functor) { onSceneEnteredFunction = FunctorScene(functor); }
            template<typename T> void setOnSceneLeftFunction(const T& functor) { onSceneLeftFunction = FunctorScene(functor); }
        };


        template <typename TEntity> class ECSSystem<TEntity> {
            protected:
                func_update         _SUF;
                func_component      _CAE;
                func_entity         _EAS;
                func_scene          _SEF;
                func_scene          _SLF;
            public:
                ECSSystem() = default;
                virtual ~ECSSystem() = default;

                virtual void update(const float& dt) {}
                virtual void onComponentAddedToEntity(void*) {}
                virtual void onEntityAddedToScene(Entity&) {}
                virtual void onSceneLeft(Scene& _Scene) {}
                virtual void onSceneEntered(Scene& _Scene) {}
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> final : public ECSSystem<TEntity> {
            using super     = ECSSystem<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;

                void _SUF(const FunctorUpdate& _functor) { super::_SUF = boost::bind(_functor.functor, &componentPool, _2); }
                void _CAE(const FunctorComponent& _functor) { super::_CAE = boost::bind(_functor.functor, _1); }
                void _EAS(const FunctorEntity& _functor) { super::_EAS = boost::bind(_functor.functor, &componentPool, _2); }
                void _SEF(const FunctorScene& _functor) { super::_SEF = boost::bind(_functor.functor, &componentPool, _2); }
                void _SLF(const FunctorScene& _functor) { super::_SLF = boost::bind(_functor.functor, &componentPool, _2); }
            public:
                ECSSystem() = default;
                ECSSystem(const ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
                    _SUF(_systemCI.updateFunction);
                    _CAE(_systemCI.onComponentAddedToEntityFunction);
                    _EAS(_systemCI.onEntityAddedToSceneFunction);
                    _SEF(_systemCI.onSceneEnteredFunction);
                    _SLF(_systemCI.onSceneLeftFunction);
                }
                ~ECSSystem() = default;

                ECSSystem(const ECSSystem&) = default;
                ECSSystem& operator=(const ECSSystem&) = default;
                ECSSystem(ECSSystem&& other) noexcept = default;
                ECSSystem& operator=(ECSSystem&& other) noexcept = default;

                void update(const float& dt) { super::_SUF(&componentPool,dt); }
                void onComponentAddedToEntity(void* _component) { super::_CAE(_component); }
                void onEntityAddedToScene(Entity& _entity) { super::_EAS(&componentPool, _entity); }
                void onSceneEntered(Scene& _Scene) { super::_SEF(&componentPool, _Scene); }
                void onSceneLeft(Scene& _Scene) { super::_SLF(&componentPool, _Scene); }
        };
    };
};
#endif