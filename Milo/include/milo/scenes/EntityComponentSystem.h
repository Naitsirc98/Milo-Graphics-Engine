#pragma once

#include "milo/common/Common.h"
#include <entt.hpp>

namespace milo {

	using ECSRegistry = entt::registry;
	using EntityId = entt::entity;

	template<typename Component>
	using ECSComponentView = entt::basic_view<EntityId, entt::exclude_t<>, Component>;

	template<typename... Component>
	using ECSComponentGroup = entt::basic_group<EntityId, entt::exclude_t<>, entt::get_t<>, Component...>;

	const EntityId NULL_ENTITY = entt::null;

	class Entity;
}