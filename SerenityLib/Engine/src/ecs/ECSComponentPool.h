#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <ecs/SparseSet.h>
#include <ecs/EntityDataRequest.h>

namespace Engine {
namespace epriv {

    template<typename ...> class ECSComponentPool;

    template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public SparseSet<TEntity, TComponent>{
        using super  = SparseSet<TEntity, TComponent>;
        public:
            ECSComponentPool() = default;
            ECSComponentPool(const ECSComponentPool& other) noexcept = delete;
            ECSComponentPool& operator=(const ECSComponentPool& other) noexcept = delete;
            ECSComponentPool(ECSComponentPool&& other) noexcept = delete;
            ECSComponentPool& operator=(ECSComponentPool&& other) noexcept = delete;

            ~ECSComponentPool() = default;

            template<typename... ARGS> inline TComponent* addComponent(const TEntity& _entity, ARGS&&... _args) {
				const EntityDataRequest dataRequest(_entity);
                return super::_add(dataRequest.ID, const_cast<TEntity&>(_entity), std::forward<ARGS>(_args)...);
            }
            inline bool removeComponent(const TEntity& _entity) {
				const EntityDataRequest dataRequest(_entity);
                return super::_remove(dataRequest.ID);
            }
            inline bool removeComponent(const EntityDataRequest& dataRequest) {
                return super::_remove(dataRequest.ID);
            }
            inline bool removeComponent(const uint& _index) {
                return super::_remove(_index); 
            }
            inline TComponent* getComponent(const TEntity& _entity) {
				const EntityDataRequest dataRequest(_entity);
                return super::_get(dataRequest.ID);
            }
            inline TComponent* getComponent(const EntityDataRequest& dataRequest) {
                return super::_get(dataRequest.ID);
            }
            inline TComponent* getComponent(const uint& _index) { 
                return super::_get(_index); 
            }
    };
};
};
#endif