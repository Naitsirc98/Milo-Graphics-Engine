#pragma once

#include <utility>

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;

	struct VulkanPhysicalDeviceInfo {
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		explicit VulkanPhysicalDeviceInfo(VkPhysicalDevice physicalDevice);

		VkPhysicalDeviceProperties properties() const;
		VkPhysicalDeviceFeatures features() const;
		VkPhysicalDeviceMemoryProperties memoryProperties() const;
		ArrayList<VkQueueFamilyProperties> queueFamilyProperties() const;
		VkPhysicalDeviceLimits limits() const;
		ArrayList<VkExtensionProperties> extensions() const;
		ArrayList<VkLayerProperties> layers() const;
		uint32_t uniformBufferAlignment() const;
		uint32_t storageBufferAlignment() const;
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

	class VulkanQueue {
		friend class VulkanDevice;
	private:
		VulkanDevice* m_Device = nullptr;
		String m_Name;
		VkQueue m_VkQueue = VK_NULL_HANDLE;
		VkQueueFlags m_Type = VK_QUEUE_FLAG_BITS_MAX_ENUM;
		uint32_t m_Family = UINT32_MAX;
		uint32_t m_Index = UINT32_MAX;
		ArrayList<VkSemaphore> m_LastSignalSemaphores;
		VkFence m_LastFence{VK_NULL_HANDLE};
	private:
		VulkanQueue() = default;
		inline void init(VulkanDevice* device, String name, VkQueueFlags type) {
			m_Device = device;
			m_Name = std::move(name);
			m_Type = type;
			m_LastSignalSemaphores.reserve(4);
		}
	public:
		~VulkanQueue() = default;
		inline VulkanDevice* device() const { return m_Device; }
		inline const String& name() const { return m_Name; }
		inline VkQueue vkQueue() const { return m_VkQueue; }
		inline VkQueueFlags type() const { return m_Type; }
		inline uint32_t family() const { return m_Family; }
		inline uint32_t index() const { return m_Index; }
		inline const ArrayList<VkSemaphore>& waitSemaphores() const { return m_LastSignalSemaphores;}
		inline void setWaitSemaphores(VkSemaphore* semaphores, uint32_t count) {
			m_LastSignalSemaphores.clear();
			for(uint32_t i = 0;i < count;++i) m_LastSignalSemaphores.push_back(semaphores[i]);
		}
		inline VkFence lastFence() const {return m_LastFence;}
		inline void setFence(VkFence fence) {m_LastFence = fence;}

		void submit(const VkSubmitInfo& submitInfo, VkFence fence);
		void awaitTermination();
		void waitForFences();
		void clear();

		inline bool operator==(const VulkanQueue& rhs) const { return m_Family == rhs.m_Family && m_Index == rhs.m_Index; }
		inline bool operator!=(const VulkanQueue& rhs) const { return !(*this == rhs); }
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
		VulkanContext* m_Context;
		VkPhysicalDevice m_Physical = VK_NULL_HANDLE;
		VkDevice m_Logical = VK_NULL_HANDLE;
		VulkanQueue m_GraphicsQueue;
		VulkanQueue m_ComputeQueue;
		VulkanQueue m_TransferQueue;
		VulkanQueue m_PresentationQueue;
		VulkanCommandPool* m_GraphicsCommandPool;
		VulkanCommandPool* m_ComputeCommandPool;
		VulkanCommandPool* m_TransferCommandPool;
	private:
		explicit VulkanDevice(VulkanContext* context);
		void init(const VulkanDevice::Info& info);
	public:
		~VulkanDevice();
		void awaitTermination();
		void awaitTermination(VkQueue queue);
		VulkanContext* context() const;
		VkPhysicalDevice physical() const;
		VkDevice logical() const;
		VulkanQueue* graphicsQueue();
		VulkanQueue* computeQueue();
		VulkanQueue* transferQueue();
		VulkanQueue* presentationQueue();
		VulkanQueue* queue(VkQueueFlags type);
		VulkanPhysicalDeviceInfo info() const;
		String name() const;
		VkFormat depthFormat() const;
		VulkanCommandPool* graphicsCommandPool() const;
		VulkanCommandPool* computeCommandPool() const;
		VulkanCommandPool* transferCommandPool() const;

		bool operator==(const VulkanDevice& rhs) const;
		bool operator!=(const VulkanDevice& rhs) const;

	private:
		ArrayList<VkDeviceQueueCreateInfo> inferQueueCreateInfos(const VulkanDevice::Info &info);
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