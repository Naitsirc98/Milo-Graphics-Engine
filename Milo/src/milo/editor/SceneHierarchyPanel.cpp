#include "milo/editor/SceneHierarchyPanel.h"
#include "milo/editor/UIRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/assets/AssetManager.h"
#include "milo/editor/MiloEditor.h"
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

	void SceneHierarchyPanel::selectEntity(const Entity& entity) {
		m_SelectedEntity = entity;
		for(auto& callback : m_SelectedCallbacks) {
			callback(entity);
		}
	}

	void SceneHierarchyPanel::unselect() {
		m_SelectedEntity = {};
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
			}
		}

		handleDragDrop(windowRect);

		handlePopupMenu(scene);

		ImGui::End();
	}

	void SceneHierarchyPanel::handlePopupMenu(Scene* scene) {

		if (ImGui::BeginPopupContextWindow(nullptr, 1, false))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Empty"))
				{
					auto newEntity = scene->createEntity("Empty");
					selectEntity(newEntity);
				}
				if (ImGui::MenuItem("Camera"))
				{
					auto newEntity = scene->createEntity("Camera");
					newEntity.addComponent<Camera>();
					selectEntity(newEntity);
				}
				if (ImGui::BeginMenu("3D"))
				{
					if (ImGui::MenuItem("Cube"))
					{
						createEntityWithMesh(scene, "Cube", Assets::meshes().getCube());
					}
					if (ImGui::MenuItem("Sphere"))
					{
						createEntityWithMesh(scene, "Sphere", Assets::meshes().getSphere());
					}
					if (ImGui::MenuItem("Cylinder"))
					{
						createEntityWithMesh(scene, "Cylinder", Assets::meshes().getCylinder());
					}
					if (ImGui::MenuItem("Plane"))
					{
						createEntityWithMesh(scene, "Plane", Assets::meshes().getPlane());
					}
					if (ImGui::MenuItem("Monkey"))
					{
						createEntityWithMesh(scene, "Monkey", Assets::meshes().getMonkey());
					}
					if (ImGui::MenuItem("Sponza"))
					{
						Model* model = Assets::models().find("Sponza");
						createEntityModelTree(scene, model, model->root(), scene->createEntity(model->root()->name));
					}
					ImGui::EndMenu();
				}

				ImGui::Separator();

				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	void SceneHierarchyPanel::createEntityWithMesh(Scene* scene, const String& name, Mesh* mesh) {
		auto newEntity = scene->createEntity(name);
		newEntity.setName(name);
		MeshView& meshView = newEntity.addComponent<MeshView>();
		meshView.mesh = mesh;
		meshView.material = Assets::materials().getDefault();
		selectEntity(newEntity);
	}

	void SceneHierarchyPanel::createEntityModelTree(Scene* scene, Model* model, const Model::Node* node, Entity entity) {

		Transform& transform = entity.getComponent<Transform>();
		transform.setMatrix(node->transform);
		transform.scale *= 0.1f;

		if(node->mesh != nullptr) {
			MeshView& meshView = entity.addComponent<MeshView>();
			meshView.mesh = node->mesh;
			meshView.material = node->material;
		}

		for(uint32_t i = 0;i < node->children.size();++i) {
			const Model::Node* childNode = model->get(node->children[i]);
			Entity childEntity = scene->createEntity(childNode->name);
			entity.addChild(childEntity.id());
			createEntityModelTree(scene, model, childNode, childEntity);
		}
	}

	void SceneHierarchyPanel::handleDragDrop(const ImRect& windowRect) {
		if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID)) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchyPanel", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload) {
				Entity& entity = *(Entity*)payload->Data;
				if(entity.hasParent()) entity.parent().removeChild(entity.id());
			}

			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyPanel::drawEntityNode(Entity& entity, const EntityBasicInfo& info) {

		if(!entity.valid()) return;

		const String& name = info.name();

		ImGuiTreeNodeFlags flags = (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		if (entity.children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.id(), flags, name.c_str());

		if (ImGui::IsItemClicked()) {
			selectEntity(entity);
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
			MiloEditor::camera().setPosition(entity.getComponent<Transform>().translation);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::Text(name.c_str());
			ImGui::SetDragDropPayload("SceneHierarchyPanel", &entity, sizeof(Entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget()) {
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
				unselect();
			}
			for(auto& callback : m_DeletedCallbacks) {
				callback(entity);
			}
		}
	}
}