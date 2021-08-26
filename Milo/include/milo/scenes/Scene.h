#pragma once

#include "EntityComponentSystem.h"
#include "milo/scenes/Components.h"

namespace milo {

	class Scene {
		friend class SceneManager;
		friend class Entity;
	private:
		const String m_Name;
		ECSRegistry m_Registry;
		EntityId m_MainCameraEntity = NULL_ENTITY;
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
		Entity cameraEntity() noexcept;
		void setMainCamera(EntityId id) noexcept;

		template<typename Component>
		ECSComponentView<Component> view() {
			return m_Registry.view<Component>();
		}

		template<typename... Components>
		ECSComponentGroup<Components...> group() {
			return m_Registry.group<Components...>();
		}

	private:
		ECSRegistry& registry() noexcept;
	private:
		void update();
		void lateUpdate();
		void render();
		void renderUI();
	};
}