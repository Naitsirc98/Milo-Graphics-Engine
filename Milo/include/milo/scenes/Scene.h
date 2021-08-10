#pragma once

#include "milo/common/Common.h"
#include <entt.hpp>

namespace milo {

	class Scene {
		friend class SceneManager;
		friend class Entity;
	private:
		entt::registry m_ECSRegistry;
	public:

	};
}