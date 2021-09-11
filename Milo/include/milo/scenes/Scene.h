#pragma once

#include "EntityComponentSystem.h"
#include "milo/scenes/Components.h"
#include "milo/assets/skybox/Skybox.h"
#include "milo/lights/Light.h"

namespace milo {

	struct LightEnvironment {
		DirectionalLight directionalLight;
		ArrayList<PointLight> pointLights{};
		Color ambientColor = {0.2f, 0.2f, 0.2f, 1.0f};
	};

	class Scene {
		friend class SceneManager;
		friend class Entity;
		friend class MiloEditor;
	private:
		const String m_Name;
		ECSRegistry m_Registry;
		EntityId m_MainCameraEntity = NULL_ENTITY;
		EntityId m_SkyEntity = NULL_ENTITY;
		LightEnvironment m_LightEnvironment{};
		Viewport m_Viewport{};
		bool m_Focused = false;
	private:
		explicit Scene(const String& name);
		explicit Scene(String&& name);
		~Scene();
	public:
		const String& name() const noexcept;
		bool isActiveScene() const;
		Entity createEntity(const String& name = "Unnamed");
		bool exists(EntityId entityId) const;
		Entity find(EntityId id) const;
		void destroyEntity(EntityId entityId) noexcept;
		Entity cameraEntity() noexcept;
		Camera* camera() noexcept;
		void setMainCamera(EntityId id) noexcept;
		SkyboxView* skyboxView() const;
		void setSkyEntity(EntityId id);
		const LightEnvironment& lightEnvironment() const noexcept;
		LightEnvironment& lightEnvironment() noexcept;
		const Viewport& viewport() const noexcept;
		Size viewportSize() const noexcept;
		bool focused() const;

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
		void setFocused(bool focused);
	};
}