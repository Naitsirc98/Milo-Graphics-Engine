#include "milo/graphics/vulkan/images/VulkanSamplerMap.h"
#include <boost/container_hash/hash.hpp>

namespace milo {

	inline static VkSamplerCreateInfo createDefaultSamplerInfo() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.mipLodBias = 0;
		return samplerInfo;
	}

	const VkSamplerCreateInfo VulkanSamplerMap::DEFAULT_SAMPLER = createDefaultSamplerInfo();

	VulkanSamplerMap::VulkanSamplerMap(VulkanDevice& device) : m_Device(device) {
		m_Samplers.reserve(32);
	}

	VulkanSamplerMap::~VulkanSamplerMap() {
		for(const auto& [hash, sampler] : m_Samplers) {
			VK_CALLV(vkDestroySampler(m_Device.ldevice(), sampler, nullptr));
		}
	}

	VkSampler VulkanSamplerMap::get(const VkSamplerCreateInfo& samplerInfo) {

		size_t hash = hashOf(samplerInfo);

		if(m_Samplers.find(hash) != m_Samplers.end()) return m_Samplers[hash];

		VkSampler vkSampler;
		VK_CALL(vkCreateSampler(m_Device.ldevice(), &samplerInfo, nullptr, &vkSampler));

		m_Samplers[hash] = vkSampler;
		return vkSampler;
	}

	VkSampler VulkanSamplerMap::getDefault() {
		return get(DEFAULT_SAMPLER);
	}

	size_t VulkanSamplerMap::hashOf(const VkSamplerCreateInfo& value) noexcept {
		size_t hash = static_cast<size_t>(value.sType);
		boost::hash_combine(hash, value.magFilter);
		boost::hash_combine(hash, value.minFilter);
		boost::hash_combine(hash, value.addressModeU);
		boost::hash_combine(hash, value.addressModeV);
		boost::hash_combine(hash, value.addressModeW);
		boost::hash_combine(hash, value.anisotropyEnable);
		boost::hash_combine(hash, value.maxAnisotropy);
		boost::hash_combine(hash, value.borderColor);
		boost::hash_combine(hash, value.unnormalizedCoordinates);
		boost::hash_combine(hash, value.compareEnable);
		boost::hash_combine(hash, value.compareOp);
		boost::hash_combine(hash, value.mipmapMode);
		boost::hash_combine(hash, value.minLod);
		boost::hash_combine(hash, value.maxLod);
		boost::hash_combine(hash, value.mipLodBias);
		return hash;
	}
}