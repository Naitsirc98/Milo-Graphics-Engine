#pragma once

#include "milo/common/Common.h"
#include "milo/scenes/Scene.h"
#include "milo/scenes/components/Tag.h"
#include "milo/scenes/components/Transform.h"

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
		T* getComponent() const noexcept {
			return m_Scene.m_Registry.try_get<T>(m_Id);
		}

		template<typename T, typename ...Args>
		T* addComponent(Args&& ...args) {
			return m_Scene.registry().get_or_emplace<T>(std::forward<>(args)...);
		}
	};
}