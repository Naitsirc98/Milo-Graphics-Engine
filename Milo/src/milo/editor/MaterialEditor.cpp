#include "milo/editor/MaterialEditor.h"
#include "milo/input/Input.h"

namespace milo {

	template<typename T>
	class Modifiable {
	private:
		T m_Value;
		bool m_Modified = false;
	public:
		Modifiable() = default;
		Modifiable(const T& initialValue) : m_Value(initialValue) {}

		Modifiable& operator=(const T& value) {
			set(value);
			return *this;
		}

		const T& get() const {return m_Value;}

		void set(const T& value) {
			if(m_Value != value) {
				m_Value = value;
				m_Modified = true;
			}
		}

		bool isModified() const {return m_Modified;}

		void reset() {m_Modified = false;}
	};

	struct MaterialNodeData {

		Modifiable<Color> albedo{Colors::WHITE};
		Modifiable<Color> emissiveColor{Colors::WHITE};
		// Values
		Modifiable<float> alpha{1.0f};
		Modifiable<float> metallic{1.0f};
		Modifiable<float> roughness{1.0f};
		Modifiable<float> occlusion{1.0f};
		// Textures
		Modifiable<String> albedoMap;
		Modifiable<String> metallicMap;
		Modifiable<String> roughnessMap;
		Modifiable<String> occlusionMap;
		Modifiable<String> emissiveMap;
		Modifiable<String> normalMap;

		bool isModified() const {
			return albedo.isModified() || emissiveColor.isModified() || alpha.isModified()
				|| metallic.isModified() || roughness.isModified() || occlusion.isModified()
				|| albedoMap.isModified() || metallicMap.isModified() || roughnessMap.isModified()
				|| occlusionMap.isModified() || emissiveMap.isModified() || normalMap.isModified();
		}

		void reset() {
			(albedo = Colors::WHITE).reset();
			(emissiveColor = Colors::WHITE).reset();
			(alpha = 1.0f).reset();
			(metallic = 1.0f).reset();
			(roughness = 1.0f).reset();
			(occlusion = 1.0f).reset();
			(albedoMap = "").reset();
			(metallicMap = "").reset();
			(roughnessMap = "").reset();
			(occlusionMap = "").reset();
			(emissiveMap = "").reset();
			(normalMap = "").reset();
		}
	};

	static const int32_t MATERIAL_NODE_ID = 10101010;
	static const int32_t PIN_ALBEDO_ID = MATERIAL_NODE_ID + 1;
	static const int32_t PIN_EMISSIVE_ID = MATERIAL_NODE_ID + 2;
	static const int32_t PIN_ALPHA_ID = MATERIAL_NODE_ID + 3;
	static const int32_t PIN_METALLIC_ID = MATERIAL_NODE_ID + 4;
	static const int32_t PIN_ROUGHNESS_ID = MATERIAL_NODE_ID + 5;
	static const int32_t PIN_OCCLUSION_ID = MATERIAL_NODE_ID + 6;
	static const int32_t PIN_ALBEDO_TEXTURE_ID = MATERIAL_NODE_ID + 7;
	static const int32_t PIN_EMISSIVE_TEXTURE_ID = MATERIAL_NODE_ID + 8;
	static const int32_t PIN_NORMAL_TEXTURE_ID = MATERIAL_NODE_ID + 9;
	static const int32_t PIN_METALLIC_TEXTURE_ID = MATERIAL_NODE_ID + 10;
	static const int32_t PIN_ROUGHNESS_TEXTURE_ID = MATERIAL_NODE_ID + 11;
	static const int32_t PIN_OCCLUSION_TEXTURE_ID = MATERIAL_NODE_ID + 12;

	static MaterialNodeData g_MaterialData{};

	MaterialEditor::MaterialEditor() {

		ed::Config config;

		config.UserPointer = &m_BlueprintBuilder;

		config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t {

			bp::BlueprintBuilder* bb = (bp::BlueprintBuilder*) userPointer;

			auto node = bb->findNode(nodeId);
			if (!node) return 0;

			if (data != nullptr) memcpy(data, node->state.data(), node->state.size());

			return node->state.size();
		};

		config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool {

			bp::BlueprintBuilder* bb = (bp::BlueprintBuilder*) userPointer;

			auto node = bb->findNode(nodeId);
			if (!node) return false;

			node->state.assign(data, size);

			bb->touchNode(nodeId);

			return true;
		};

		m_Context = ed::CreateEditor(&config);
		//ed::SetCurrentEditor(m_Context);

		//createIcon(m_BlueprintBuilder.getSaveIcon(), "");
		//createIcon(m_BlueprintBuilder.getRestoreIcon(), "");

		m_DockSpaceRenderer.setDockSpaceWindowName("MaterialEditorDockSpaceWindow");
		m_DockSpaceRenderer.setDockSpaceName("MaterialEditorDockSpace");
		m_DockSpaceRenderer.setOnFirstFrame([&](ImGuiID dockSpaceId) {

			ImGuiID left = NULL;
			ImGuiID right = NULL;

			// Split screen into left and right sections
			ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Up, 0.25f, &left, &right);

			ImGui::DockBuilderDockWindow("MaterialViewer", left);
			ImGui::DockBuilderDockWindow("MaterialNodeEditor", right);

			ImGui::DockBuilderFinish(dockSpaceId);
		});
	}

	MaterialEditor::~MaterialEditor() {

		auto* node = m_BlueprintBuilder.findNode(MATERIAL_NODE_ID);
		DELETE_PTR(node);

		DELETE_PTR(m_MaterialViewerRenderer);
		ed::SetCurrentEditor(nullptr);
		ed::DestroyEditor(m_Context);
	}

	bool MaterialEditor::isOpen() const {
		return m_Open;
	}

	void MaterialEditor::setOpen(bool open) {
		m_Open = open;
	}

	static bp::Node getMaterialNode() {

		static bool firstTime = true;
		static int id = MATERIAL_NODE_ID;
		static bp::Node node(id, "Material");

		if(firstTime) {
			node.inputs.emplace_back(PIN_ALBEDO_ID, "Albedo color", bp::PinType::Float3);
			node.inputs.emplace_back(PIN_EMISSIVE_ID, "Emissive color", bp::PinType::Float3);
			node.inputs.emplace_back(PIN_ALPHA_ID, "Alpha value", bp::PinType::Float);
			node.inputs.emplace_back(PIN_METALLIC_ID, "Metallic value", bp::PinType::Float);
			node.inputs.emplace_back(PIN_ROUGHNESS_ID, "Roughness value", bp::PinType::Float);
			node.inputs.emplace_back(PIN_OCCLUSION_ID, "Occlusion value", bp::PinType::Float);

			node.inputs.emplace_back(PIN_ALBEDO_TEXTURE_ID, "Albedo Texture", bp::PinType::Texture2D);
			node.inputs.emplace_back(PIN_EMISSIVE_TEXTURE_ID, "Emissive Texture", bp::PinType::Texture2D);
			node.inputs.emplace_back(PIN_NORMAL_TEXTURE_ID, "Normal Texture", bp::PinType::Texture2D);
			node.inputs.emplace_back(PIN_METALLIC_TEXTURE_ID, "Metallic Texture", bp::PinType::Texture2D);
			node.inputs.emplace_back(PIN_ROUGHNESS_TEXTURE_ID, "Roughness Texture", bp::PinType::Texture2D);
			node.inputs.emplace_back(PIN_OCCLUSION_TEXTURE_ID, "Occlusion Texture", bp::PinType::Texture2D);
		}

		return node;
	}

	void MaterialEditor::render(Material* material) {
		if(material == nullptr) return;

		if(m_LastMaterial != nullptr && material != m_LastMaterial) {
			g_MaterialData.reset();
			m_BlueprintBuilder.reset();
		}

		if(m_MaterialViewerRenderer == nullptr) {

			m_MaterialViewerRenderer = MaterialViewerRenderer::create();

			bp::Icon headerBackground{};
			createIcon(headerBackground, "resources\\textures\\tree\\Bark_Color.png");
			m_BlueprintBuilder.setHeaderBackground(headerBackground);

			m_BlueprintBuilder.addNode(getMaterialNode());
		}

		auto& io = ImGui::GetIO();

		bool openThisFrame = m_Open;

		if(openThisFrame && ImGui::Begin("Material Editor", &m_Open)) {

			renderMaterialViewer(material);

			ImGui::Separator();

			ed::SetCurrentEditor(m_Context);

			m_BlueprintBuilder.draw();

			setMaterialData(material);

			if (m_FirstFrame)
				ed::NavigateToContent(0.0f);

			ed::SetCurrentEditor(nullptr);

			m_FirstFrame = false;
		}

		if(openThisFrame) ImGui::End();
	}

	void MaterialEditor::beginColumn() {
		ImGui::BeginGroup();
	}

	void MaterialEditor::nextColumn() {
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
	}

	void MaterialEditor::endColumn() {
		ImGui::EndGroup();
	}

	void MaterialEditor::createIcon(bp::Icon& icon, const String& textureFile) {

		Image* image = Image::loadImage(textureFile, PixelFormat::RGBA8);

		Ref<Texture2D> texture = Ref<Texture2D>(Texture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_UI_BIT));

		Texture2D::AllocInfo allocInfo{};
		allocInfo.format = image->format();
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.pixels = image->pixels();

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		icon.texture = texture;
		icon.iconId = UI::getIconId(*texture);
	}

	void MaterialEditor::renderMaterialViewer(Material* material) {

		ImGui::Begin("MaterialViewer");

		//TODO: real time render of the material
		//auto icon = Assets::textures().getIcon("MaterialViewerIcon");
		//if(icon == nullptr) {
		//	icon = Assets::textures().createIcon("MaterialViewerIcon", Assets::meshes().getSphere(), material);
		//}
		//UI::image(*icon);

		const auto& texture = m_MaterialViewerRenderer->render(material);
		UI::image(texture, {1920, 1080});

		//ImGui::Separator();
		//ImGui::Text("Name: %s", material->name().c_str());
//
		//ImGui::Separator();
		//ImGui::Text("File: %s", material->filename().c_str());

		ImGui::End();

		//if(material->dirty()) { // TODO: recreate icon when material is modified
		//	Assets::textures().removeIcon("MaterialViewerIcon");
		//	Assets::textures().createIcon("MaterialViewerIcon", Assets::meshes().getSphere(), material);
		//}
	}

	int32_t MaterialEditor::getNextId() {
		return m_NextId++;
	}

	void MaterialEditor::setMaterialData(Material* material) {

		fetchMaterialDataFromNodes();

		if(g_MaterialData.isModified()) {

			if(g_MaterialData.albedo.isModified()) {
				material->albedo(g_MaterialData.albedo.get());
				g_MaterialData.albedo.reset();
			}

			if(g_MaterialData.emissiveColor.isModified()) {
				material->emissiveColor(g_MaterialData.emissiveColor.get());
				g_MaterialData.emissiveColor.reset();
			}

			if(g_MaterialData.metallic.isModified()) {
				material->metallic(g_MaterialData.metallic.get());
				g_MaterialData.metallic.reset();
			}

			if(g_MaterialData.roughness.isModified()) {
				material->roughness(g_MaterialData.roughness.get());
				g_MaterialData.roughness.reset();
			}

			if(g_MaterialData.occlusion.isModified()) {
				material->occlusion(g_MaterialData.occlusion.get());
				g_MaterialData.occlusion.reset();
			}

			if(g_MaterialData.albedoMap.isModified()) {
				material->albedoMap(Assets::textures().load(g_MaterialData.albedoMap.get()));
				g_MaterialData.albedoMap.reset();
			}

			if(g_MaterialData.emissiveMap.isModified()) {
				material->emissiveMap(Assets::textures().load(g_MaterialData.emissiveMap.get()));
				g_MaterialData.emissiveMap.reset();
			}

			if(g_MaterialData.normalMap.isModified()) {
				material->normalMap(Assets::textures().load(g_MaterialData.normalMap.get()));
				g_MaterialData.normalMap.reset();
			}

			if(g_MaterialData.metallicMap.isModified()) {
				material->metallicMap(Assets::textures().load(g_MaterialData.metallicMap.get()));
				g_MaterialData.metallicMap.reset();
			}

			if(g_MaterialData.roughnessMap.isModified()) {
				material->roughnessMap(Assets::textures().load(g_MaterialData.roughnessMap.get()));
				g_MaterialData.roughnessMap.reset();
			}

			if(g_MaterialData.occlusionMap.isModified()) {
				material->occlusionMap(Assets::textures().load(g_MaterialData.occlusionMap.get()));
				g_MaterialData.occlusionMap.reset();
			}
		}
	}

	void MaterialEditor::fetchMaterialDataFromNodes() {

		for(const auto& link : m_BlueprintBuilder.links()) {

			// Get links that ends at the material node

			const auto* startPin = m_BlueprintBuilder.findPin(link.startPinID);
			const auto* endPin = m_BlueprintBuilder.findPin(link.endPinID);

			if(startPin == nullptr || endPin == nullptr) continue;
			if(endPin->node->id != ed::NodeId(MATERIAL_NODE_ID)) continue;

			const bp::Node* srcNode = startPin->node;

			Vector3 color;
			float value;
			String str;

			switch(endPin->id.Get()) {
				case PIN_ALBEDO_ID:
					color = ((Vector3*)srcNode->output())[0];
					g_MaterialData.albedo = Color(color, 1);
					break;
				case PIN_EMISSIVE_ID:
					color = ((Vector3*)srcNode->output())[0];
					g_MaterialData.emissiveColor = Color(color, 1);
					break;
				case PIN_ALPHA_ID:
					value = ((float*)srcNode->output())[0];
					g_MaterialData.alpha = value;
					break;
				case PIN_METALLIC_ID:
					value = ((float*)srcNode->output())[0];
					g_MaterialData.metallic = value;
					break;
				case PIN_ROUGHNESS_ID:
					value = ((float*)srcNode->output())[0];
					g_MaterialData.roughness = value;
					break;
				case PIN_OCCLUSION_ID:
					value = ((float*)srcNode->output())[0];
					g_MaterialData.occlusion = value;
					break;
				case PIN_ALBEDO_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.albedoMap = str;
					break;
				case PIN_EMISSIVE_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.emissiveMap = str;
					break;
				case PIN_NORMAL_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.normalMap = str;
					break;
				case PIN_METALLIC_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.metallicMap = str;
					break;
				case PIN_ROUGHNESS_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.roughnessMap = str;
					break;
				case PIN_OCCLUSION_TEXTURE_ID:
					str = ((char*)srcNode->output());
					g_MaterialData.occlusionMap = str;
					break;
			}
		}
	}
}