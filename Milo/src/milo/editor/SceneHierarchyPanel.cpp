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

	void SceneHierarchyPanel::render() {

		ImGui::Begin("SceneHierarchyPanel");

		Scene* scene = SceneManager::activeScene();

		auto entities = scene->view<EntityBasicInfo>();
		for(EntityId entityId : entities) {
			Entity entity = scene->find(entityId);
			const EntityBasicInfo& info = entities.get<EntityBasicInfo>(entityId);
			if(info.parentId() == NULL_ENTITY) {
				drawEntityNode(entity, info);
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(const Entity& entity, const EntityBasicInfo& info) {

		if(!entity.valid()) return;

		const char* name = info.name().c_str();

		if(entity.hasComponent<Tag>()) {
			name = entity.getComponent<Tag>().value();
		}

		ImGuiTreeNodeFlags flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entity.children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		// TODO(Peter): This should probably be a function that checks that the entities components are valid
		//const bool missingMesh = entity.hasComponent<MeshView>()
		//        && (entity.getComponent<MeshView>().mesh
		//		&& entity.getComponent<MeshView>().mesh->IsFlagSet(AssetFlag::Missing));
		//if (missingMesh)
		//	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.3f, 1.0f));

		//bool isPrefab = entity.hasComponent<PrefabComponent>();
		//if (isPrefab)
		//	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.32f, 0.7f, 0.87f, 1.0f));
		const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.id(), flags, name);
		//if (isPrefab)
		//	ImGui::PopStyleColor();
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
			//if (m_SelectionChangedCallback)
				//m_SelectionChangedCallback(m_SelectionContext);
		}

		//if (missingMesh)
		//	ImGui::PopStyleColor();

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

			if (payload)
			{
				//Entity& droppedEntity = *(Entity*)payload->Data;
				//m_Context->ParentEntity(droppedEntity, entity);
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
			if(entity.id() == m_SelectionContext.id()) {
				m_SelectionContext = {};
			}
			//m_EntityDeletedCallback(entity);
		}
	}
}