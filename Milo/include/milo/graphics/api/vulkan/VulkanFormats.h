#pragma once

#include "milo/common/Collections.h"
#include <vulkan/vulkan.h>

namespace milo::VulkanFormats {

	VkFormat findSupportedFormat(VkPhysicalDevice device, const ArrayList<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlagBits features);
	VkFormat findDepthFormat(VkPhysicalDevice device);
	bool formatHasStencilComponent(VkFormat format);
}