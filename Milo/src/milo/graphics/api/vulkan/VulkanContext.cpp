#include "milo/graphics/api/vulkan/VulkanContext.h"

namespace milo {

	VulkanContext::VulkanContext() {
	}

	VulkanContext::~VulkanContext() {
		DELETE_PTR(m_Device);

		vkDestroyInstance(m_VkInstance, a);
	}

	Handle VulkanContext::handle() const {
		return vkInstance();
	}

	VkInstance VulkanContext::vkInstance() const {
#ifdef _DEBUG
		if(m_VkInstance == VK_NULL_HANDLE) throw MILO_EXCEPTION("VkInstance has not been initialized!");
#endif
		return m_VkInstance;
	}

	VulkanDevice& VulkanContext::device() const {
#ifdef _DEBUG
		if(m_Device == nullptr) throw MILO_EXCEPTION("Device has not been initialized!");
#endif
		return *m_Device;
	}

	void VulkanContext::init() {
		createVkInstance();
		createMainVulkanDevice();
	}

	void VulkanContext::createVkInstance() {

	}

	void VulkanContext::createMainVulkanDevice() {

	}
}