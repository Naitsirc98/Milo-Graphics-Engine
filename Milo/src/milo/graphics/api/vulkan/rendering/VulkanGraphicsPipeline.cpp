#include "milo/graphics/api/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/api/rendering/SPIRVCompiler.h"

namespace milo {

	VkPipeline VulkanGraphicsPipeline::create(const String& name, VkDevice device, const VulkanGraphicsPipelineInfo& info) {

#ifdef _DEBUG
		if(info.vkPipelineLayout == VK_NULL_HANDLE) throw MILO_RUNTIME_EXCEPTION("Pipeline Layout has not been set");
		if(info.vkRenderPass == VK_NULL_HANDLE) throw MILO_RUNTIME_EXCEPTION("Render Pass has not been set");
		if(info.shaderInfos.empty()) throw MILO_RUNTIME_EXCEPTION("Graphics Pipeline has no shaders");
		Log::debug("Creating GraphicsPipeline {}", name);
#endif

		ArrayList<VkShaderModule> shaderModules = toShaderModules(device, info.shaderInfos);
		ArrayList<VkPipelineShaderStageCreateInfo> shaderStages = createShaderPipelineStages(info.shaderInfos, shaderModules);

		VkPipelineVertexInputStateCreateInfo vertexInputState = createVertexInputState(info.vertexInputInfo);

		const VkPipelineInputAssemblyStateCreateInfo& assemblyStateInfo = info.inputAssembly;

		const VkPipelineDepthStencilStateCreateInfo& depthStencil = info.depthStencil;

		const VkPipelineViewportStateCreateInfo& viewportStateInfo = info.viewportState;

		const VkPipelineRasterizationStateCreateInfo& rasterizationStateInfo = info.rasterizationState;

		const VkPipelineMultisampleStateCreateInfo& multisampleStateInfo = info.multisampleState;

		const VkPipelineColorBlendStateCreateInfo& colorBlendStateInfo = info.colorBlendState;

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
		pipelineInfo.layout = info.vkPipelineLayout;
		pipelineInfo.renderPass = info.vkRenderPass;
		pipelineInfo.subpass = info.renderSubPass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		float start = Time::millis();

		VkPipeline vkPipeline;
		VK_CALL(vkCreateGraphicsPipelines(device, info.vkPipelineCache, 1, &pipelineInfo, nullptr, &vkPipeline));

		Log::debug("vkCreateGraphicsPipelines finished after {} ms", Time::millis() - start);

		for(VkShaderModule shaderModule : shaderModules) {
			VK_CALLV(vkDestroyShaderModule(device, shaderModule, nullptr));
		}

		return vkPipeline;
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

	inline shaderc_shader_kind getShaderStage(const VulkanShaderInfo& shaderInfo) {
		switch(shaderInfo.stage) {
			case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_vertex_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_fragment_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_compute_shader;
			case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_tess_control_shader;
			case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_tess_evaluation_shader;
		}
		throw MILO_RUNTIME_EXCEPTION(str("Unknown shader type ") + str(shaderInfo.stage));
	}

	VkShaderModule VulkanGraphicsPipeline::createShaderModule(VkDevice device, const VulkanShaderInfo& shaderInfo) {

		SPIRVCompiler compiler;
		SPIRV spirv = compiler.compile(shaderInfo.filename, getShaderStage(shaderInfo));

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.length();
		createInfo.pCode = spirv.code();

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

	VulkanGraphicsPipelineInfo::VulkanGraphicsPipelineInfo(VkPipelineLayout pipelineLayout, VkRenderPass renderPass, uint32_t subPass)
		: vkPipelineLayout(pipelineLayout), vkRenderPass(renderPass), renderSubPass(subPass) {

		initVulkanVertexInputInfo();
		initInputAssembly();
		initDepthStencil();
		initViewportState();
		initRasterizationState();
		initMultisampleState();
		initColorBlendAttachmentState();
		initColorBlendState();
	}

	void VulkanGraphicsPipelineInfo::initVulkanVertexInputInfo() {

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

	void VulkanGraphicsPipelineInfo::initInputAssembly() {
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void VulkanGraphicsPipelineInfo::initDepthStencil() {
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	}

	void VulkanGraphicsPipelineInfo::initViewportState() {
		const Size windowSize = Window::get().size();

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

	void VulkanGraphicsPipelineInfo::initRasterizationState() {
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.lineWidth = 1.0f;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthBiasEnable = VK_FALSE;
	}

	void VulkanGraphicsPipelineInfo::initMultisampleState() {
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanGraphicsPipelineInfo::initColorBlendAttachmentState() {
		VkPipelineColorBlendAttachmentState attachmentState = {};
		attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		attachmentState.blendEnable = VK_FALSE;

		colorBlendAttachments.push_back(attachmentState);
	}

	void VulkanGraphicsPipelineInfo::initColorBlendState() {
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