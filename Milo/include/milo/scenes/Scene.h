#pragma once

#include "milo/common/Common.h"
#include <entt.hpp>

namespace milo {

	using ECSRegistry = entt::registry;
	using EntityId = entt::entity;

	const EntityId NULL_ENTITY = entt::null;

	class Entity;

	class Scene {
		friend class SceneManager;
		friend class Entity;
	private:
		const String m_Name;
		ECSRegistry m_Registry;
	private:
		explicit Scene(const String& name);
		explicit Scene(String&& name);
		~Scene();
	public:
		const String& name() const noexcept;
		bool isActiveScene() const;
		Entity createEntity();
		bool exists(EntityId entityId) const;
		Entity find(EntityId id) const;
		void destroyEntity(EntityId entityId) noexcept;
	private:
		ECSRegistry& registry() noexcept;
	private:
		void update();
		void lateUpdate();
		void render();
		void renderUI();
	};
}