#pragma once

#include <imgui/imgui_internal.h>
#include "milo/scenes/Scene.h"
#include "milo/scenes/Entity.h"
#include "milo/assets/AssetManager.h"

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
		void selectEntity(const Entity& entity);
		void unselect();
		void addSelectedCallback(EntitySelectedCallback callback);
		void addDeletedCallback(EntityDeletedCallback callback);
	private:
		void drawEntityNode(Entity& entity, const EntityBasicInfo& info);
		void handleDragDrop(const ImRect& windowRect);
		void handlePopupMenu(Scene* scene);
		void createEntityWithMesh(Scene* scene, const String& name, Mesh* mesh);
		void createEntityModelTree(Scene* scene, Model* model, const Model::Node* node, Entity entity);
	};
}