#pragma once

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;

	struct VulkanPhysicalDeviceInfo {
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		explicit VulkanPhysicalDeviceInfo(VkPhysicalDevice physicalDevice);

		[[nodiscard]] VkPhysicalDeviceProperties properties() const;
		[[nodiscard]] VkPhysicalDeviceFeatures features() const;
		[[nodiscard]] VkPhysicalDeviceMemoryProperties memoryProperties() const;
		[[nodiscard]] ArrayList<VkQueueFamilyProperties> queueFamilyProperties() const;
		[[nodiscard]] VkPhysicalDeviceLimits limits() const;
		[[nodiscard]] ArrayList<VkExtensionProperties> extensions() const;
		[[nodiscard]] ArrayList<VkLayerProperties> layers() const;
	};

	enum DeviceUsageFlags {
		DeviceUsageGraphicsBit = 0x1,
		DeviceUsageTransferBit = 0x2,
		DeviceUsageComputeBit = 0x4,
		DeviceUsagePresentationBit = 0x8,
		DeviceUsageAllBit = -1
	};

	using DeviceScore = int64_t;

	struct RankedDevice {
		VkPhysicalDevice physicalDevice;
		DeviceScore score;

		bool operator<(const RankedDevice &rhs) const;
		bool operator>(const RankedDevice &rhs) const;
		bool operator<=(const RankedDevice &rhs) const;
		bool operator>=(const RankedDevice &rhs) const;
	};

	struct VulkanDeviceQueue {
		VkQueue vkQueue = VK_NULL_HANDLE;
		uint32_t family = UINT32_MAX;
		uint32_t index = 0;
	};

	class VulkanDevice {
		friend class VulkanContext;
	public:
		struct Info {
			VkPhysicalDevice physicalDevice;
			DeviceUsageFlags usageFlags = DeviceUsageAllBit;
			ArrayList<const char*> extensionNames;
			ArrayList<const char*> layerNames;
		};
	private:
		inline static const float QUEUE_PRIORITY = 1.0f;
		inline static const float GRAPHICS_QUEUE_PRIORITY = 1.0f;
		inline static const float TRANSFER_QUEUE_PRIORITY = 1.0f;
		inline static const float COMPUTE_QUEUE_PRIORITY = 1.0f;
		inline static const float PRESENTATION_QUEUE_PRIORITY = 1.0f;
	private:
		VulkanContext& m_Context;
		VkPhysicalDevice m_Pdevice = VK_NULL_HANDLE;
		VkDevice m_Ldevice = VK_NULL_HANDLE;
		VulkanDeviceQueue m_GraphicsQueue = {};
		VulkanDeviceQueue m_ComputeQueue = {};
		VulkanDeviceQueue m_TransferQueue = {};
		VulkanDeviceQueue m_PresentationQueue = {};
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
		[[nodiscard]] const VulkanDeviceQueue& graphicsQueue() const;
		[[nodiscard]] const VulkanDeviceQueue& computeQueue() const;
		[[nodiscard]] const VulkanDeviceQueue& transferQueue() const;
		[[nodiscard]] const VulkanDeviceQueue& presentationQueue() const;
		[[nodiscard]] VulkanPhysicalDeviceInfo pDeviceInfo() const;
		[[nodiscard]] String name() const;
	public:
		static ArrayList<VkPhysicalDevice> listAllPhysicalDevices(VkInstance vkInstance);
		static ArrayList<RankedDevice> rankAllPhysicalDevices(VkInstance vkInstance);
		static void rankDeviceProperties(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceFeatures(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceMemory(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceQueueFamilies(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static ArrayList <VkDeviceQueueCreateInfo> inferQueueCreateInfos(const VulkanDevice::Info &info);

		static void tryGetQueue(VkQueueFlagBits queueType,
								const Info &info,
								const ArrayList<VkQueueFamilyProperties> &queueFamilies, ArrayList<VkDeviceQueueCreateInfo> &queues);
		static void tryGetPresentationQueue(const Info &info, const ArrayList<VkQueueFamilyProperties> &queueFamilies, ArrayList<VkDeviceQueueCreateInfo> &queues);

		static uint32_t findBestQueueFamilyOf(VkQueueFlagBits queueType, const ArrayList<VkQueueFamilyProperties> &queueFamilies);
	};
}