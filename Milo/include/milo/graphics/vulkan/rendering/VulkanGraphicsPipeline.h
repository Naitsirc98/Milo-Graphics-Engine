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

	class VulkanGraphicsPipeline {
	public:

		struct CreateInfo {

			VkRenderPass vkRenderPass = VK_NULL_HANDLE;
			uint32_t renderSubPass = 0;
			ArrayList<VkPushConstantRange> pushConstantRanges;
			ArrayList<VkDescriptorSetLayout> setLayouts;
			VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
			ArrayList<VulkanShaderInfo> shaders;
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
			ArrayList<VkDynamicState> dynamicStates;
		public:
			explicit CreateInfo();
			~CreateInfo() = default;
			void setVertexAttributes(Vertex::Attribute attributes);
		private:
			void initVulkanVertexInputInfo(Vertex::Attribute attributes);
			void initInputAssembly();
			void initDepthStencil();
			void initViewportState();
			void initRasterizationState();
			void initMultisampleState();
			void initColorBlendAttachmentState();
			void initColorBlendState();
		};
	private:
		VulkanDevice* m_Device{nullptr};
		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_Pipeline{VK_NULL_HANDLE};
		VkPipelineCache m_PipelineCache{VK_NULL_HANDLE};
	public:
		VulkanGraphicsPipeline(const String& name, VulkanDevice* device, const CreateInfo& info);
		~VulkanGraphicsPipeline();
		VulkanDevice* device() const;
		VkPipelineLayout pipelineLayout() const;
		VkPipeline vkPipeline() const;
		VkPipelineCache pipelineCache() const;
	private:
		static ArrayList<VkPipelineShaderStageCreateInfo> createShaderPipelineStages(const ArrayList<VulkanShaderInfo>& shaderInfos, const ArrayList<VkShaderModule>& shaderModules);
		static VkShaderModule createShaderModule(VkDevice device, const VulkanShaderInfo& shaderInfo);
		static ArrayList<VkShaderModule> toShaderModules(VkDevice device, const ArrayList<VulkanShaderInfo>& shaderInfos);
		static VkPipelineVertexInputStateCreateInfo createVertexInputState(const VulkanVertexInputInfo& vertexInfo);
	};
}