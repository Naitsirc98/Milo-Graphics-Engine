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
		[[nodiscard]] uint32_t uniformBufferAlignment() const;
		[[nodiscard]] uint32_t storageBufferAlignment() const;
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

	class VulkanDevice;

	struct VulkanQueue {
		VulkanDevice* device = nullptr;
		const char* name = "";
		VkQueue vkQueue = VK_NULL_HANDLE;
		VkQueueFlags type = VK_QUEUE_FLAG_BITS_MAX_ENUM;
		uint32_t family = UINT32_MAX;
		uint32_t index = 0;

		inline bool operator==(const VulkanQueue &rhs) const {
			return device == rhs.device && family == rhs.family && index == rhs.index;
		}

		inline bool operator!=(const VulkanQueue &rhs) const {
			return !(rhs == *this);
		}

		void awaitTermination() const;
	};

	class VulkanCommandPool;

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
	private:
		VulkanContext& m_Context;
		VkPhysicalDevice m_Pdevice = VK_NULL_HANDLE;
		VkDevice m_Ldevice = VK_NULL_HANDLE;
		VulkanQueue m_GraphicsQueue = {};
		VulkanQueue m_ComputeQueue = {};
		VulkanQueue m_TransferQueue = {};
		VulkanQueue m_PresentationQueue = {};
		VulkanCommandPool* m_TransferCommandPool = nullptr;
	private:
		explicit VulkanDevice(VulkanContext& m_Context);
		~VulkanDevice();
		void init(const VulkanDevice::Info& info);
	public:
		void awaitTermination();
		void awaitTermination(VkQueue queue);
		[[nodiscard]] VulkanContext& context() const;
		[[nodiscard]] VkPhysicalDevice pdevice() const;
		[[nodiscard]] VkDevice ldevice() const;
		[[nodiscard]] const VulkanQueue& graphicsQueue() const;
		[[nodiscard]] const VulkanQueue& computeQueue() const;
		[[nodiscard]] const VulkanQueue& transferQueue() const;
		[[nodiscard]] const VulkanQueue& presentationQueue() const;
		[[nodiscard]] VulkanPhysicalDeviceInfo pDeviceInfo() const;
		[[nodiscard]] String name() const;
		[[nodiscard]] VkFormat depthFormat() const;
		[[nodiscard]] VulkanCommandPool* transferCommandPool() const;
	private:
		ArrayList <VkDeviceQueueCreateInfo> inferQueueCreateInfos(const VulkanDevice::Info &info);
		void tryGetQueue(VkQueueFlagBits queueType, const Info &info, const ArrayList<VkQueueFamilyProperties> &queueFamilies, ArrayList<VkDeviceQueueCreateInfo> &queues);
		void tryGetPresentationQueue(const Info &info, const ArrayList<VkQueueFamilyProperties> &queueFamilies, ArrayList<VkDeviceQueueCreateInfo> &queues);
		void getQueues();
	public:
		static ArrayList<VkPhysicalDevice> listAllPhysicalDevices(VkInstance vkInstance);
		static ArrayList<RankedDevice> rankAllPhysicalDevices(VkInstance vkInstance);
		static void rankDeviceProperties(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceFeatures(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceMemory(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static void rankDeviceQueueFamilies(const VulkanPhysicalDeviceInfo &info, DeviceScore &score);
		static uint32_t findBestQueueFamilyOf(VkQueueFlagBits queueType, const ArrayList<VkQueueFamilyProperties> &queueFamilies);
	};
}