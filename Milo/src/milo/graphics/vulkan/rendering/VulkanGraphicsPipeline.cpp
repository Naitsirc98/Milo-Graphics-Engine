#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"

namespace milo {

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const String& name, VulkanDevice* device, const VulkanGraphicsPipeline::CreateInfo& info) {

		m_Device = device;

#ifdef _DEBUG
		if(info.vkRenderPass == VK_NULL_HANDLE) throw MILO_RUNTIME_EXCEPTION("Render Pass has not been set");
		if(info.shaders.empty()) throw MILO_RUNTIME_EXCEPTION("Graphics Pipeline has no shaders");
#endif

		VkPipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pSetLayouts = info.setLayouts.data();
		layoutCreateInfo.setLayoutCount = info.setLayouts.size();
		layoutCreateInfo.pPushConstantRanges = info.pushConstantRanges.data();
		layoutCreateInfo.pushConstantRangeCount = info.pushConstantRanges.size();

		VK_CALL(vkCreatePipelineLayout(device->logical(), &layoutCreateInfo, nullptr, &m_PipelineLayout));

		ArrayList<VkShaderModule> shaderModules = toShaderModules(device->logical(), info.shaders);
		ArrayList<VkPipelineShaderStageCreateInfo> shaderStages = createShaderPipelineStages(info.shaders, shaderModules);

		VkPipelineVertexInputStateCreateInfo vertexInputState = createVertexInputState(info.vertexInputInfo);

		const VkPipelineInputAssemblyStateCreateInfo& assemblyStateInfo = info.inputAssembly;

		const VkPipelineDepthStencilStateCreateInfo& depthStencil = info.depthStencil;

		const VkPipelineViewportStateCreateInfo& viewportStateInfo = info.viewportState;

		const VkPipelineRasterizationStateCreateInfo& rasterizationStateInfo = info.rasterizationState;

		const VkPipelineMultisampleStateCreateInfo& multisampleStateInfo = info.multisampleState;

		const VkPipelineColorBlendStateCreateInfo& colorBlendStateInfo = info.colorBlendState;

		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.pDynamicStates = info.dynamicStates.data();
		dynamicStateInfo.dynamicStateCount = info.dynamicStates.size();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &assemblyStateInfo;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizationStateInfo;
		pipelineInfo.pMultisampleState = &multisampleStateInfo;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = info.vkRenderPass;
		pipelineInfo.subpass = info.renderSubPass;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		float start = Time::millis();

		VK_CALL(vkCreateGraphicsPipelines(device->logical(), info.vkPipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline));

		Log::debug("{} pipeline created after {} ms", name, Time::millis() - start);

		for(VkShaderModule shaderModule : shaderModules) {
			VK_CALLV(vkDestroyShaderModule(device->logical(), shaderModule, nullptr));
		}

		m_PipelineCache = info.vkPipelineCache;
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {

		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_Pipeline, nullptr));

		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));
	}

	VulkanDevice* VulkanGraphicsPipeline::device() const {
		return m_Device;
	}

	VkPipelineLayout VulkanGraphicsPipeline::pipelineLayout() const {
		return m_PipelineLayout;
	}

	VkPipeline VulkanGraphicsPipeline::vkPipeline() const {
		return m_Pipeline;
	}

	VkPipelineCache VulkanGraphicsPipeline::pipelineCache() const {
		return m_PipelineCache;
	}

	ArrayList<VkPipelineShaderStageCreateInfo> VulkanGraphicsPipeline::createShaderPipelineStages(
			const ArrayList<VulkanShaderInfo>& shaderInfos,
			const ArrayList<VkShaderModule>& shaderModules) {

		ArrayList<VkPipelineShaderStageCreateInfo> shaderStages(shaderInfos.size());
		for(uint32_t i = 0;i < shaderInfos.size();++i) {
			VkPipelineShaderStageCreateInfo& shaderStage = shaderStages[i];
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = shaderInfos[i].stage;
			shaderStage.module = shaderModules[i];
			shaderStage.pName = "main";
		}

		return shaderStages;
	}

	VkShaderModule VulkanGraphicsPipeline::createShaderModule(VkDevice device, const VulkanShaderInfo& shaderInfo) {

		VulkanShader* shader = dynamic_cast<VulkanShader*>(Assets::shaders().load(shaderInfo.filename));

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader->bytecodeLength();
		createInfo.pCode = (uint32_t*)shader->bytecode();

		VkShaderModule shaderModule;
		VK_CALL(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	ArrayList<VkShaderModule> VulkanGraphicsPipeline::toShaderModules(VkDevice device, const ArrayList<VulkanShaderInfo>& shaderInfos) {
		ArrayList<VkShaderModule> shaderModules;
		shaderModules.reserve(shaderInfos.size());
		for(const auto& shaderInfo : shaderInfos) {
			shaderModules.push_back(createShaderModule(device, shaderInfo));
		}
		return shaderModules;
	}

	VkPipelineVertexInputStateCreateInfo VulkanGraphicsPipeline::createVertexInputState(const VulkanVertexInputInfo& vertexInfo) {
		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.pVertexAttributeDescriptions = vertexInfo.attributes.data();
		vertexInputState.vertexAttributeDescriptionCount = vertexInfo.attributes.size();
		vertexInputState.pVertexBindingDescriptions = vertexInfo.bindings.data();
		vertexInputState.vertexBindingDescriptionCount = 1;
		return vertexInputState;
	}

	// ======

	VulkanGraphicsPipeline::CreateInfo::CreateInfo() {

		initVulkanVertexInputInfo();
		initInputAssembly();
		initDepthStencil();
		initViewportState();
		initRasterizationState();
		initMultisampleState();
		initColorBlendAttachmentState();
		initColorBlendState();
	}

	void VulkanGraphicsPipeline::CreateInfo::initVulkanVertexInputInfo() {

		VkVertexInputBindingDescription binding = {};
		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex3D);

		VkVertexInputAttributeDescription positionAttribute = {};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.offset = offsetof(Vertex3D, position);
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

		VkVertexInputAttributeDescription normalAttribute = {};
		normalAttribute.binding = 0;
		normalAttribute.location = 1;
		normalAttribute.offset = offsetof(Vertex3D, normal);
		normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

		VkVertexInputAttributeDescription texCoordsAttribute = {};
		texCoordsAttribute.binding = 0;
		texCoordsAttribute.location = 2;
		texCoordsAttribute.offset = offsetof(Vertex3D, texCoords);
		texCoordsAttribute.format = VK_FORMAT_R32G32_SFLOAT;

		vertexInputInfo.bindings.reserve(1);
		vertexInputInfo.bindings.push_back(binding);

		vertexInputInfo.attributes.reserve(3);
		vertexInputInfo.attributes.push_back(positionAttribute);
		vertexInputInfo.attributes.push_back(normalAttribute);
		vertexInputInfo.attributes.push_back(texCoordsAttribute);
	}

	void VulkanGraphicsPipeline::CreateInfo::initInputAssembly() {
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void VulkanGraphicsPipeline::CreateInfo::initDepthStencil() {
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	}

	void VulkanGraphicsPipeline::CreateInfo::initViewportState() {

		const Size windowSize = Window::get()->size();

		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(windowSize.width);
		viewport.height = static_cast<float>(windowSize.height);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		scissor.offset = {0, 0};
		scissor.extent = {(uint32_t)windowSize.width, (uint32_t)windowSize.height};

		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pScissors = &scissor;
		viewportState.scissorCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.viewportCount = 1;
	}

	void VulkanGraphicsPipeline::CreateInfo::initRasterizationState() {
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.lineWidth = 1.0f;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthBiasEnable = VK_FALSE;
	}

	void VulkanGraphicsPipeline::CreateInfo::initMultisampleState() {
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanGraphicsPipeline::CreateInfo::initColorBlendAttachmentState() {
		VkPipelineColorBlendAttachmentState attachmentState = {};
		attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		attachmentState.blendEnable = VK_FALSE;

		colorBlendAttachments.push_back(attachmentState);
	}

	void VulkanGraphicsPipeline::CreateInfo::initColorBlendState() {
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = colorBlendAttachments.size();
		colorBlendState.pAttachments = colorBlendAttachments.data();
		colorBlendState.blendConstants[0] = 0.0f;
		colorBlendState.blendConstants[1] = 0.0f;
		colorBlendState.blendConstants[2] = 0.0f;
		colorBlendState.blendConstants[3] = 0.0f;
	}

}