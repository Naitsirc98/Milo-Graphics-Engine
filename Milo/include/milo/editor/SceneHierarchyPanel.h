#pragma once

#include "milo/scenes/Scene.h"
#include "milo/scenes/Entity.h"

namespace milo {

	class SceneHierarchyPanel {
	private:
		Entity m_SelectionContext = {NULL_ENTITY, nullptr};
	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();
	public:
		void render();
	private:
		void drawEntityNode(const Entity& entity, const EntityBasicInfo& info);
	};
}