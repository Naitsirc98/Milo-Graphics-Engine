#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	struct VulkanShaderInfo {
		String filename;
		VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct VulkanVertexInputInfo {
		ArrayList<VkVertexInputBindingDescription> bindings;
		ArrayList<VkVertexInputAttributeDescription> attributes;
	};

	struct VulkanGraphicsPipelineInfo {

		VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
		VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
		VkRenderPass vkRenderPass = VK_NULL_HANDLE;
		uint32_t renderSubPass = 0;
		ArrayList<VulkanShaderInfo> shaderInfos;
		VulkanVertexInputInfo vertexInputInfo = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		VkViewport viewport = {};
		VkRect2D scissor = {};
		VkPipelineViewportStateCreateInfo viewportState = {};
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		ArrayList<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};

	public:
		explicit VulkanGraphicsPipelineInfo(VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, VkRenderPass renderPass = VK_NULL_HANDLE, uint32_t subPass = 0);
		~VulkanGraphicsPipelineInfo() = default;
	private:
		void initVulkanVertexInputInfo();
		void initInputAssembly();
		void initDepthStencil();
		void initViewportState();
		void initRasterizationState();
		void initMultisampleState();
		void initColorBlendAttachmentState();
		void initColorBlendState();
	};

	class VulkanGraphicsPipeline {
	public:
		static VkPipeline create(const String& name, VkDevice device, const VulkanGraphicsPipelineInfo& info);
	private:
		static ArrayList<VkPipelineShaderStageCreateInfo> createShaderPipelineStages(const ArrayList<VulkanShaderInfo>& shaderInfos, const ArrayList<VkShaderModule>& shaderModules);
		static VkShaderModule createShaderModule(VkDevice device, const VulkanShaderInfo& shaderInfo);
		static ArrayList<VkShaderModule> toShaderModules(VkDevice device, const ArrayList<VulkanShaderInfo>& shaderInfos);
		static VkPipelineVertexInputStateCreateInfo createVertexInputState(const VulkanVertexInputInfo& vertexInfo);
	};
}