#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanSamplerMap {
	public:
		static const VkSamplerCreateInfo DEFAULT_SAMPLER;
	private:
		VulkanDevice* m_Device;
		HashMap<size_t, VkSampler> m_Samplers;
	public:
		explicit VulkanSamplerMap(VulkanDevice* device);
		~VulkanSamplerMap();
		VkSampler get(const VkSamplerCreateInfo& samplerInfo);
		VkSampler getDefault();
	private:
		static size_t hashOf(const VkSamplerCreateInfo& value) noexcept;
	};
}