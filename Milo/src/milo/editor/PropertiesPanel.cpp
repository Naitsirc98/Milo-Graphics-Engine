#include "milo/editor/PropertiesPanel.h"
#include "milo/editor/UIRenderer.h"
#include "milo/assets/AssetManager.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace milo {

	PropertiesPanel::PropertiesPanel() = default;

	PropertiesPanel::~PropertiesPanel() = default;

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
	static String nameof() {
		static const size_t offset = strlen("class milo::");
		const String name = typeid(T).name();
		return name.substr(offset);
	}

	template<typename T>
	static void addComponentButton(Entity entity) {
		if(!entity.hasComponent<T>()) {
			String name = nameof<T>();
			if(ImGui::Button(name.c_str()))
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
			addComponentButton<SphereCollider>(entity);
			addComponentButton<BoxCollider>(entity);
			addComponentButton<DirectionalLight>(entity);
			addComponentButton<PointLight>(entity);
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

		drawComponent<Transform>("Transform", entity, [](Transform& transform) {

			Vector3 translation = transform.translation();
			Vector3 scale = transform.scale();
			Vector3 rotation = milo::eulerAngles(transform.rotation()) * 180.0f / MILO_PI; // To degrees

			drawVector3Control("Translation", translation);
			drawVector3Control("Scale", scale, 1.0f);
			drawVector3Control("Rotation", rotation);

			transform.translation(translation);
			transform.scale(scale);
			transform.rotation(Quaternion(rotation * MILO_PI / 180.0f)); // To radians

		}, false);

		drawComponent<MeshView>("MeshView", entity, [this](MeshView& meshView) {

			Mesh* mesh = meshView.mesh;
			ImGui::Text("Mesh");
			if(mesh != nullptr) {
				if(mesh->icon()) {
					UI::image(*mesh->icon());
				} else {
					UI::image(*Assets::textures().getIcon("DefaultMeshIcon"));
				}
				ImGui::SameLine();
				ImGui::Text(mesh->name().c_str());
			}
			if(ImGui::Button("Select Mesh")) {
				auto file = UI::FileDialog::open("*.obj;*.fbx;*.gltf;*.collada");
				if(file.has_value()) {
					String& filename = file.value();
					mesh = Assets::meshes().load(Files::getName(filename, true), filename);
					meshView.mesh = mesh;
				}
			}

			ImGui::Separator();

			Material* material = meshView.material;
			ImGui::Text("Material");
			if(material != nullptr) {
				if(mesh->icon()) {
					UI::image(*material->icon());
				} else {
					UI::image(*Assets::textures().getIcon("DefaultMaterialIcon"));
				}
				ImGui::SameLine();
				ImGui::Text(material->name().c_str());
			}
			if(ImGui::Button("Select Material")) {
				auto file = UI::FileDialog::open("*.mat");
				if(file.has_value()) {
					if(file.has_value()) {
						String& filename = file.value();
						material = Assets::materials().load(Files::getName(filename, true), filename, true);
						meshView.material = material;
					}
				}
			}
		});

		drawComponent<SkyboxView>("SkyboxView", entity, [](SkyboxView& skyboxView) {

			if(skyboxView.type == SkyType::Static) {

				ImGui::Text("Skybox");
				IMGUI_LEFT_LABEL(ImGui::Text, "Name: ", skyboxView.skybox->name().c_str());

				float maxPrefilterLod = skyboxView.skybox->maxPrefilterLOD();
				float prefilterLodBias = skyboxView.skybox->prefilterLODBias();

				if(ImGui::DragFloat("Max Prefilter LOD", &maxPrefilterLod, 0.05f, 0.0f)) {
					skyboxView.skybox->maxPrefilterLOD(maxPrefilterLod);
				}
				if(ImGui::DragFloat("Prefilter LOD Bias", &prefilterLodBias, 0.05f)) {
					skyboxView.skybox->prefilterLODBias(prefilterLodBias);
				}

			} else {

				PreethamSky* sky = (PreethamSky*)skyboxView.skybox;
				float turbidity = sky->turbidity();
				float azimuth = sky->azimuth();
				float inclination = degrees(sky->inclination());

				ImGui::Text("Preetham sky");

				float maxPrefilterLod = skyboxView.skybox->maxPrefilterLOD();
				float prefilterLodBias = skyboxView.skybox->prefilterLODBias();

				if(ImGui::DragFloat("Max Prefilter LOD", &maxPrefilterLod, 0.05f, 0.0f)) {
					skyboxView.skybox->maxPrefilterLOD(maxPrefilterLod);
				}
				if(ImGui::DragFloat("Prefilter LOD Bias", &prefilterLodBias, 0.05f)) {
					skyboxView.skybox->prefilterLODBias(prefilterLodBias);
				}

				if(ImGui::DragFloat("Turbidity", &turbidity, 0.01f, 0.0f)) {
					sky->turbidity(turbidity);
				}
				if(ImGui::DragFloat("Azimuth", &azimuth, 0.1f, 0.0f)) {
					sky->azimuth(azimuth);
				}
				if(ImGui::DragFloat("Inclination", &inclination, 0.5f, 0.0f, 360.0f)) {
					sky->inclination(radians(inclination));
				}
			}
		});

		drawComponent<SphereCollider>("SphereCollider", entity, [](SphereCollider& collider) {
			drawVector3Control("Center", collider.sphere.center);
			ImGui::DragFloat("Radius", &collider.sphere.radius, 0);
		});

		drawComponent<BoxCollider>("BoxCollider", entity, [](BoxCollider& collider) {
			drawVector3Control("Center", collider.box.center);
			drawVector3Control("Size", collider.box.size);
			drawVector3Control("X Axis", collider.box.xAxis);
			drawVector3Control("Y Axis", collider.box.yAxis);
			drawVector3Control("Z Axis", collider.box.zAxis);
		});

		drawComponent<DirectionalLight>("Directional Light", entity, [](DirectionalLight& light) {
			drawVector3Control("Direction", light.direction);
			drawVector3Control("Color", light.color);
			ImGui::DragFloat("Multiplier", &light.multiplier, 0, FLT_MAX, 0.1f);
			ImGui::Checkbox("Casts Shadows", &light.castShadows);
		});

		drawComponent<PointLight>("Point Light", entity, [&](PointLight& light) {
			Transform& transform = entity.getComponent<Transform>();

			Vector3 position = transform.translation();
			Vector3 color = Vector3(light.color);

			drawVector3Control("Position", position);
			drawVector3Control("Color", color);
			ImGui::DragFloat("Multiplier", &light.multiplier, 0, FLT_MAX, 0.1f);
			ImGui::DragFloat("Radius", &light.radius, 0, FLT_MAX, 0.1f);
			ImGui::DragFloat("Fall off", &light.falloff, 0, FLT_MAX, 0.1f);
			ImGui::DragFloat("Min Radius", &light.minRadius, 0, FLT_MAX, 0.1f);
			ImGui::DragFloat("Source size", &light.sourceSize, 0, FLT_MAX, 0.1f);
			//ImGui::Checkbox("Casts Shadows", &light.castsShadows);

			light.position = Vector4(position, 1.0f);
			light.color = Vector4(color, 1.0f);

			transform.translation(position);
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