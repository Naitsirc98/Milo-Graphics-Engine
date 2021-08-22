#pragma once

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
			return getComponent<T>() != nullptr;
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
			return m_Scene.registry().emplace<T>(m_Id, std::forward<Args>(args)...);
		}
	};
}