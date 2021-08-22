#pragma once

#include <milo/logging/Log.h>
#include "milo/common/Common.h"
#include "milo/scenes/Scene.h"
#include "Components.h"

namespace milo {

	class Entity {
		friend class Scene;
	private:
		const EntityId m_Id;
		Scene& m_Scene;
	private:
		Entity(EntityId id, Scene& scene);
	public:
		EntityId id() const noexcept;
		Scene& scene() const noexcept;
		bool valid() const noexcept;
		void destroy() noexcept;

		template<typename T>
		bool hasComponent() const noexcept {
			return tryGetComponent<T>() != nullptr;
		}

		template<typename T>
		T& getComponent() const noexcept {
			return m_Scene.m_Registry.get<T>(m_Id);
		}

		template<typename T>
		T* tryGetComponent() const noexcept {
			return m_Scene.m_Registry.try_get<T>(m_Id);
		}

		template<typename T, typename ...Args>
		T& createComponent(Args&& ...args) {
			if(hasComponent<T>()) return getComponent<T>();
			return m_Scene.registry().emplace<T>(m_Id, std::forward<Args>(args)...);
		}

		template<typename T>
		void destroyComponent() {
			m_Scene.m_Registry.erase<T>(m_Id);
		}

		template<>
		void destroyComponent<Transform>() {
			Log::error("Cannot destroy transform component");
		}

		bool operator==(const Entity& rhs) const;
		bool operator!=(const Entity& rhs) const;

	};
}