#pragma once

#include "milo/common/Collections.h"
#include "VulkanDevice.h"

namespace milo {

	namespace VulkanExtensions {
		ArrayList<const char*> getInstanceExtensions();
		ArrayList<const char*> getDeviceExtensions(DeviceUsageFlags usageFlags);
	}

	namespace VulkanLayers {
		ArrayList<const char*> getInstanceLayers();
		ArrayList<const char*> getDeviceLayers(DeviceUsageFlags usageFlags);
	}
}