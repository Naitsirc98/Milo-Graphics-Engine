#include "milo/graphics/vulkan/VulkanFormats.h"
#include "milo/logging/Log.h"

namespace milo::VulkanFormats {

	VkFormat findSupportedFormat(VkPhysicalDevice device, const ArrayList<VkFormat>& formats,
												VkImageTiling tiling,
												VkFormatFeatureFlagBits features) {
		VkFormatProperties properties = {};

		for (const VkFormat format : formats) {
			vkGetPhysicalDeviceFormatProperties(device, format, &properties);
			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
				return format;
			if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
				return format;
		}

		LOG_ERROR("Failed to find supported format");

		return VK_FORMAT_MAX_ENUM;
	}

	VkFormat findDepthFormat(VkPhysicalDevice device) {
		return findSupportedFormat(
				device,
				{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool formatHasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}