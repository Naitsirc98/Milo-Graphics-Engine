#pragma once

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;

	struct VulkanPhysicalDeviceInfo {
		const VkPhysicalDevice physicalDevice;

		VulkanPhysicalDeviceInfo(const VkPhysicalDevice_T* physicalDevice);

		[[nodiscard]] VkPhysicalDeviceProperties properties() const;
		[[nodiscard]] VkPhysicalDeviceFeatures features() const;
		[[nodiscard]] VkPhysicalDeviceMemoryProperties memoryProperties() const;
		[[nodiscard]] ArrayList<VkQueueFamilyProperties> queueFamilyProperties() const;
		[[nodiscard]] ArrayList<VkExtensionProperties> extensions() const;
		[[nodiscard]] ArrayList<VkLayerProperties> layers() const;
	};

	class VulkanDevice {
		friend class VulkanContext;
	public:
		struct Info {
			String pdeviceName = "";
			ArrayList<String> extensionNames;
			ArrayList<String> layerNames;
		};
	private:
		VulkanContext& m_Context;
		VkPhysicalDevice m_Pdevice = VK_NULL_HANDLE;
		VkDevice m_Ldevice = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_ComputeQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		VkQueue m_PresentationQueue = VK_NULL_HANDLE;
	private:
		VulkanDevice(VulkanContext& m_Context);
		~VulkanDevice();

		void init(const VulkanDevice::Info& info);
	public:
		void waitFor();
		void waitFor(VkQueue queue);

		[[nodiscard]] VulkanContext& context() const;
		[[nodiscard]] VkPhysicalDevice pdevice() const;
		[[nodiscard]] VkDevice ldevice() const;
		[[nodiscard]] VkQueue graphicsQueue() const;
		[[nodiscard]] VkQueue computeQueue() const;
		[[nodiscard]] VkQueue transferQueue() const;
		[[nodiscard]] VkQueue presentationQueue() const;
		[[nodiscard]] VulkanPhysicalDeviceInfo pDeviceInfo() const;
	public:
		static ArrayList<VkPhysicalDevice> listAllPhysicalDevices(VkInstance vkInstance);
	};

}