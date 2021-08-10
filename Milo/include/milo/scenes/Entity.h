#pragma once

#include "milo/common/Common.h"

#include <entt.hpp>

namespace milo {

	using EntityId = entt::entity;

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
	};

}