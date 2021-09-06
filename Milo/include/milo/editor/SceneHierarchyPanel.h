#pragma once

#include "milo/scenes/Scene.h"
#include "milo/scenes/Entity.h"

namespace milo {

	using EntitySelectedCallback = Function<void, Entity>;
	using EntityDeletedCallback = Function<void, Entity>;

	class SceneHierarchyPanel {
	private:
		Entity m_SelectedEntity = {NULL_ENTITY, nullptr};
		ArrayList<EntitySelectedCallback> m_SelectedCallbacks;
		ArrayList<EntityDeletedCallback> m_DeletedCallbacks;
	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();
	public:
		void render();
		Entity selectedEntity();
		void setSelectedEntity(Entity entity);
		void addSelectedCallback(EntitySelectedCallback callback);
		void addDeletedCallback(EntityDeletedCallback callback);
	private:
		void drawEntityNode(Entity& entity, const EntityBasicInfo& info);
	};
}