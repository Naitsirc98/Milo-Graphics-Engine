#include "milo/editor/PropertiesPanel.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace milo {

	PropertiesPanel::PropertiesPanel() {

	}

	PropertiesPanel::~PropertiesPanel() {

	}

	void PropertiesPanel::render(Entity entity) {

		ImGui::Begin("PropertiesPanel");

		ImGui::AlignTextToFramePadding();

		{
			const String& name = entity.name();
			char buffer[256];
			memset(buffer, 0, 256);
			memcpy(buffer, name.c_str(), name.size());
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			if (IMGUI_LEFT_LABEL(ImGui::InputText, "Name", buffer, 256)) {
				entity.setName(buffer);
			}
			ImGui::PopItemWidth();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("ID: %llx", (uint64_t)entity.id());

		ImGui::Separator();

		handleAddComponents(entity);

		ImGui::End();
	}

	template<typename T>
	static void addComponentButton(Entity entity) {
		if(!entity.hasComponent<T>()) {
			if(ImGui::Button(typeid(T).name()))
			{
				entity.addComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	template<typename T, typename UIFunction>
	static void drawComponent(const String& name, Entity entity, UIFunction drawerFunction, bool removable = true) {

		static const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.hasComponent<T>()) {

			ImGui::PushID((void*)typeid(T).hash_code());
			auto& component = entity.getComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
			bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
			ImGui::PopStyleVar();

			bool resetValues = false;
			bool removeComponent = false;

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }) || right_clicked) {
				ImGui::OpenPopup("ComponentSettings");
			}

			if (ImGui::BeginPopup("ComponentSettings")) {
				if (ImGui::MenuItem("Reset"))
					resetValues = true;

				if (removable) {
					if (ImGui::MenuItem("Remove component")) removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open) {
				drawerFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.destroyComponent<T>();
			}

			if (resetValues) {
				if(entity.hasComponent<T>()) {
					entity.resetComponent<T>();
				} else {
					entity.addComponent<T>();
				}
			}

			ImGui::Separator();

			ImGui::PopID();
		}
	}

	void PropertiesPanel::handleAddComponents(Entity entity) {

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 textSize = ImGui::CalcTextSize("Add Component");
		ImGui::SameLine(contentRegionAvailable.x - (textSize.x + GImGui->Style.FramePadding.y));
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponentPanel");

		if(ImGui::BeginPopup("AddComponentPanel")) {
			addComponentButton<Tag>(entity);
			addComponentButton<Camera>(entity);
			addComponentButton<MeshView>(entity);
			// addComponentButton<NativeScript>(entity);
			// TODO

			ImGui::EndPopup();
		}

		drawComponent<Tag>("Tag", entity, [](Tag& tag){

			char buffer[TAG_MAX_SIZE];
			memset(buffer, 0, TAG_MAX_SIZE);
			memcpy(buffer, tag.value(), strlen(tag.value()));
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			if (IMGUI_LEFT_LABEL(ImGui::InputText, "Tag", buffer, TAG_MAX_SIZE)) {
				tag.setValue(buffer);
			}
			ImGui::PopItemWidth();

		}, false);

		drawComponent<Transform>("Transform", entity, [](Transform& transform){

			drawVector3Control("Translation", transform.translation);
			drawVector3Control("Scale", transform.scale, 1.0f);
			Vector3 rotation = milo::eulerAngles(transform.rotation);
			drawVector3Control("Rotation", rotation);
			transform.rotation = Quaternion(rotation);

		}, false);

		drawComponent<MeshView>("MeshView", entity, [](MeshView& meshView) {

			ImGui::Text("Mesh: %s", meshView.mesh == nullptr ? "" : meshView.mesh->filename().c_str());
			ImGui::Text("Material: %s", meshView.material == nullptr ? "" : meshView.material->filename().c_str());
		});
	}

	bool PropertiesPanel::drawVector3Control(const String& label, Vector3& vector, float resetValue, float columnWidth) {

		bool modified = false;

		const ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) {
			vector.x = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##X", &vector.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) {
			vector.y = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Y", &vector.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) {
			vector.z = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Z", &vector.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return modified;
	}
}