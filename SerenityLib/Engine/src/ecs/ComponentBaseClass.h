#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include <ecs/Entity.h>

class ComponentBaseClass{
	protected:
		Entity m_Owner;
	public:
        ComponentBaseClass();
        ComponentBaseClass(const Entity& p_Owner);

		ComponentBaseClass& operator=(const ComponentBaseClass& p_Other)     = delete;
		ComponentBaseClass(const ComponentBaseClass& p_Other)                = delete;
		ComponentBaseClass(ComponentBaseClass&& p_Other) noexcept            = default;
		ComponentBaseClass& operator=(ComponentBaseClass&& p_Other) noexcept = default;

		virtual ~ComponentBaseClass() = default;

        Entity& getOwner();
};

#endif