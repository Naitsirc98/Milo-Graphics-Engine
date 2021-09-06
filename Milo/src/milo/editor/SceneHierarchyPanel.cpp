#include "milo/editor/SceneHierarchyPanel.h"
#include "milo/editor/UIRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace milo {

	SceneHierarchyPanel::SceneHierarchyPanel() {

	}

	SceneHierarchyPanel::~SceneHierarchyPanel() {

	}

	Entity SceneHierarchyPanel::selectedEntity() {
		return m_SelectedEntity;
	}

	void SceneHierarchyPanel::setSelectedEntity(Entity entity) {
		m_SelectedEntity = entity;
	}

	void SceneHierarchyPanel::addSelectedCallback(EntitySelectedCallback callback) {
		if(callback) m_SelectedCallbacks.push_back(callback);
	}

	void SceneHierarchyPanel::addDeletedCallback(EntityDeletedCallback callback) {
		if(callback) m_DeletedCallbacks.push_back(callback);
	}

	void SceneHierarchyPanel::render() {

		ImGui::Begin("SceneHierarchyPanel");

		Scene* scene = SceneManager::activeScene();

		ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		auto entities = scene->view<EntityBasicInfo>();
		for(EntityId entityId : entities) {
			Entity entity = scene->find(entityId);
			const EntityBasicInfo& info = entities.get<EntityBasicInfo>(entityId);
			if(info.parentId() == NULL_ENTITY) {
				drawEntityNode(entity, info);
			} else {
				Log::info("");
			}
		}

		if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchyPanel", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload) {
				Entity& entity = *(Entity*)payload->Data;
				if(entity.hasParent()) entity.parent().removeChild(entity.id());
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(Entity& entity, const EntityBasicInfo& info) {

		if(!entity.valid()) return;

		const char* name = info.name().c_str();

		if(entity.hasComponent<Tag>()) {
			name = entity.getComponent<Tag>().value();
		}

		ImGuiTreeNodeFlags flags = (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entity.children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.id(), flags, name);

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
			for(auto& callback : m_SelectedCallbacks) {
				callback(entity);
			}
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::Text(name);
			ImGui::SetDragDropPayload("SceneHierarchyPanel", &entity, sizeof(Entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchyPanel", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload) {
				Entity& droppedEntity = *(Entity*)payload->Data;
				entity.addChild(droppedEntity.id());
			}

			ImGui::EndDragDropTarget();
		}

		if (opened) {
			for (EntityId child : entity.children()) {
				Entity e = entity.scene()->find(child);
				if (e.valid()) drawEntityNode(e, e.getComponent<EntityBasicInfo>());
			}

			ImGui::TreePop();
		}

		// Defer deletion until end of node UI
		if (entityDeleted) {
			Log::warn("Deleting {}", name);
			Scene* scene = entity.scene();
			scene->destroyEntity(entity.id());
			if(entity.id() == m_SelectedEntity.id()) {
				m_SelectedEntity = {};
			}
			for(auto& callback : m_DeletedCallbacks) {
				callback(entity);
			}
		}
	}
}