#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanSamplerMap {
		friend class VulkanContext;
	public:
		static const VkSamplerCreateInfo DEFAULT_SAMPLER;
	private:
		VulkanDevice* m_Device;
		HashMap<size_t, VkSampler> m_Samplers;
	private:
		explicit VulkanSamplerMap(VulkanDevice* device);
		~VulkanSamplerMap();
	public:
		VkSampler get(const VkSamplerCreateInfo& samplerInfo);
		VkSampler getDefaultSampler();
	private:
		static size_t hashOf(const VkSamplerCreateInfo& value) noexcept;
	public:
		static VulkanSamplerMap* get();
	};
}