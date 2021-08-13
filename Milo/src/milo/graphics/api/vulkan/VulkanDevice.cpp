#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include <algorithm>

namespace milo {

	VulkanDevice::VulkanDevice(VulkanContext& context) : m_Context(context) {
	}

	VulkanDevice::~VulkanDevice() {
		waitFor();

		m_GraphicsQueue.vkQueue = VK_NULL_HANDLE;
		m_ComputeQueue.vkQueue = VK_NULL_HANDLE;
		m_TransferQueue.vkQueue = VK_NULL_HANDLE;
		m_PresentationQueue.vkQueue = VK_NULL_HANDLE;

		vkDestroyDevice(m_Ldevice, nullptr);
		m_Ldevice = VK_NULL_HANDLE;
		m_Pdevice = VK_NULL_HANDLE;
	}

	void VulkanDevice::init(const VulkanDevice::Info& info) {

		m_Pdevice = info.physicalDevice;

		VulkanPhysicalDeviceInfo physicalDeviceInfo(m_Pdevice);
		VkPhysicalDeviceFeatures features = physicalDeviceInfo.features();
		ArrayList<VkDeviceQueueCreateInfo> queueCreateInfos = inferQueueCreateInfos(info);

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.enabledExtensionCount = info.extensionNames.size();
		createInfo.ppEnabledExtensionNames = info.extensionNames.data();
		createInfo.enabledLayerCount = info.layerNames.size();
		createInfo.ppEnabledLayerNames = info.layerNames.data();
		createInfo.pEnabledFeatures = &features;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = queueCreateInfos.size();

		VK_CALL(vkCreateDevice(m_Pdevice, &createInfo, nullptr, &m_Ldevice));
	}

	void VulkanDevice::waitFor() {
		vkDeviceWaitIdle(m_Ldevice);
	}

	void VulkanDevice::waitFor(VkQueue queue) {
		vkQueueWaitIdle(queue);
	}

	VulkanContext& VulkanDevice::context() const {
		return m_Context;
	}

	VkPhysicalDevice VulkanDevice::pdevice() const {
		return m_Pdevice;
	}

	VkDevice VulkanDevice::ldevice() const {
		return m_Ldevice;
	}

	const VulkanDeviceQueue& VulkanDevice::graphicsQueue() const {
		return m_GraphicsQueue;
	}

	const VulkanDeviceQueue& VulkanDevice::computeQueue() const {
		return m_ComputeQueue;
	}

	const VulkanDeviceQueue& VulkanDevice::transferQueue() const {
		return m_TransferQueue;
	}

	const VulkanDeviceQueue& VulkanDevice::presentationQueue() const {
		return m_PresentationQueue;
	}

	VulkanPhysicalDeviceInfo VulkanDevice::pDeviceInfo() const {
		return VulkanPhysicalDeviceInfo(m_Pdevice);
	}

	String VulkanDevice::name() const {
		return pDeviceInfo().properties().deviceName;
	}

	ArrayList<VkPhysicalDevice> VulkanDevice::listAllPhysicalDevices(VkInstance vkInstance) {
		uint32_t count;
		vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);
		ArrayList<VkPhysicalDevice> pdevices(count);
		vkEnumeratePhysicalDevices(vkInstance, &count, pdevices.data());
		return pdevices;
	}

	ArrayList<RankedDevice> VulkanDevice::rankAllPhysicalDevices(VkInstance vkInstance) {
		ArrayList<VkPhysicalDevice> devices = listAllPhysicalDevices(vkInstance);
		ArrayList<RankedDevice> rank;
		rank.reserve(devices.size());

		for(VkPhysicalDevice pdevice : devices) {
			VulkanPhysicalDeviceInfo info(pdevice);
			DeviceScore score = 0;

			rankDeviceProperties(info, score);
			rankDeviceFeatures(info, score);
			rankDeviceMemory(info, score);
			rankDeviceQueueFamilies(info, score);

			rank.push_back({pdevice, score});
		}

		std::sort(rank.begin(), rank.end(), std::greater<>());

		return rank;
	}

	void VulkanDevice::rankDeviceProperties(const VulkanPhysicalDeviceInfo &info, DeviceScore &score) {
		VkPhysicalDeviceProperties properties = info.properties();
		score += properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 10000000 : 0;
		score += properties.limits.bufferImageGranularity;
		score += properties.limits.framebufferColorSampleCounts;
		score += properties.limits.framebufferDepthSampleCounts;
		score += properties.limits.framebufferStencilSampleCounts;
		score += properties.limits.framebufferNoAttachmentsSampleCounts;
		score += properties.limits.maxBoundDescriptorSets;
		score += properties.limits.maxClipDistances;
		score += properties.limits.maxDescriptorSetSampledImages;
		score += properties.limits.maxColorAttachments;
		score += properties.limits.maxComputeSharedMemorySize;
		score += properties.limits.maxDescriptorSetStorageBuffers;
		score += properties.limits.maxPushConstantsSize;
		score += properties.limits.maxDescriptorSetUniformBuffersDynamic;
		score += properties.limits.maxDescriptorSetUniformBuffers;
		score += properties.limits.maxDrawIndirectCount;
		// etc
	}

	void VulkanDevice::rankDeviceFeatures(const VulkanPhysicalDeviceInfo &info, DeviceScore &score) {
		VkPhysicalDeviceFeatures features = info.features();
		score += features.multiDrawIndirect ? 10 : 0;
		score += features.multiViewport ? 100 : 0;
		score += features.geometryShader ? 10 : -1000000;
		score += features.tessellationShader ? 100 : 0;
		score += features.samplerAnisotropy ? 100 : 0;
		score += features.imageCubeArray ? 10 : 0;
	}

	void VulkanDevice::rankDeviceMemory(const VulkanPhysicalDeviceInfo &info, DeviceScore &score) {
		VkPhysicalDeviceMemoryProperties memProperties = info.memoryProperties();
		for(uint32_t i = 0;i < memProperties.memoryHeapCount;++i) {
			score += memProperties.memoryHeaps[i].size;
		}
	}

	void VulkanDevice::rankDeviceQueueFamilies(const VulkanPhysicalDeviceInfo &info, DeviceScore &score) {
		score += info.queueFamilyProperties().size(); // TODO: improve
	}

	ArrayList<VkDeviceQueueCreateInfo> VulkanDevice::inferQueueCreateInfos(const VulkanDevice::Info &info) {
		ArrayList<VkQueueFamilyProperties> queueFamilies = VulkanPhysicalDeviceInfo(info.physicalDevice).queueFamilyProperties();
		ArrayList<VkDeviceQueueCreateInfo> queues;
		queues.reserve(4);

		if((info.usageFlags & DeviceUsageGraphicsBit) != 0)     tryGetQueue(VK_QUEUE_GRAPHICS_BIT, info, queueFamilies, queues);
		if((info.usageFlags & DeviceUsageTransferBit) != 0)     tryGetQueue(VK_QUEUE_TRANSFER_BIT, info, queueFamilies, queues);
		if((info.usageFlags & DeviceUsageComputeBit) != 0)      tryGetQueue(VK_QUEUE_COMPUTE_BIT, info, queueFamilies, queues);
		if((info.usageFlags & DeviceUsagePresentationBit) != 0) tryGetPresentationQueue(info, queueFamilies, queues);

		return queues;
	}

	void VulkanDevice::tryGetQueue(VkQueueFlagBits queueType, const VulkanDevice::Info &info,
										   const ArrayList<VkQueueFamilyProperties> &queueFamilies,
										   ArrayList<VkDeviceQueueCreateInfo> &queues) {

		uint32_t bestQueueFamily = findBestQueueFamilyOf(queueType, queueFamilies);

		if(bestQueueFamily == UINT32_MAX) {
			throw MILO_RUNTIME_EXCEPTION(str("Device has no queueFamilies supporting queue type ") + str(queueType));
		}

		const auto alreadyFound = std::find_if(queues.begin(), queues.end(), [&](const auto& queueInfo) {
			return queueInfo.queueFamilyIndex == bestQueueFamily;
		});

		if(alreadyFound != queues.end()) return;

		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &QUEUE_PRIORITY;
		queueInfo.queueFamilyIndex = bestQueueFamily;

		queues.push_back(queueInfo);
	}

	void VulkanDevice::tryGetPresentationQueue(const VulkanDevice::Info &info,
											   const ArrayList<VkQueueFamilyProperties> &queueFamilies,
											   ArrayList<VkDeviceQueueCreateInfo> &queues) {

		VkSurfaceKHR surface = m_Context.windowSurface().vkSurface();

		uint32_t bestQueueFamily = UINT32_MAX;
		uint32_t numQueues = UINT32_MAX;

		for(uint32_t i = 0;i < queueFamilies.size();++i) {
			const VkQueueFamilyProperties& queueFamily = queueFamilies[i];

			VkBool32 supportsPresentation;
			vkGetPhysicalDeviceSurfaceSupportKHR(info.physicalDevice, i, surface, &supportsPresentation);

			if(supportsPresentation) {
				if(queueFamily.queueFlags == VK_QUEUE_GRAPHICS_BIT) {
					bestQueueFamily = i;
				} else if((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && queueFamily.queueCount < numQueues) {
					bestQueueFamily = i;
					numQueues = queueFamily.queueCount;
				}
			}
		}

		if(bestQueueFamily == UINT32_MAX) {
			throw MILO_RUNTIME_EXCEPTION("Device has no queueFamilies supporting a presentation capable queue");
		}

		const auto alreadyFound = std::find_if(queues.begin(), queues.end(), [&](const auto& queueInfo) {
			return queueInfo.queueFamilyIndex == bestQueueFamily;
		});

		if(alreadyFound != queues.end()) return;

		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &QUEUE_PRIORITY;
		queueInfo.queueFamilyIndex = bestQueueFamily;

		queues.push_back(queueInfo);
	}

	uint32_t VulkanDevice::findBestQueueFamilyOf(VkQueueFlagBits queueType, const ArrayList<VkQueueFamilyProperties> &queueFamilies) {

		uint32_t bestQueueFamily = UINT32_MAX;
		uint32_t numQueues = UINT32_MAX;

		for(uint32_t i = 0;i < queueFamilies.size();++i) {
			const VkQueueFamilyProperties& queueFamily = queueFamilies[i];

			if(queueFamily.queueFlags == queueType) {
				bestQueueFamily = i;
			} else if((queueFamily.queueFlags & queueType) != 0 && queueFamily.queueCount < numQueues) {
				bestQueueFamily = i;
				numQueues = queueFamily.queueCount;
			}
		}

		return bestQueueFamily;
	}

	VulkanPhysicalDeviceInfo::VulkanPhysicalDeviceInfo(VkPhysicalDevice physicalDevice) : physicalDevice(physicalDevice) {
	}

	VkPhysicalDeviceProperties VulkanPhysicalDeviceInfo::properties() const {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		return deviceProperties;
	}

	VkPhysicalDeviceFeatures VulkanPhysicalDeviceInfo::features() const {
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		return deviceFeatures;
	}

	VkPhysicalDeviceMemoryProperties VulkanPhysicalDeviceInfo::memoryProperties() const {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
		return memoryProperties;
	}

	ArrayList<VkQueueFamilyProperties> VulkanPhysicalDeviceInfo::queueFamilyProperties() const {
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
		ArrayList<VkQueueFamilyProperties> queueFamilyProperties(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());
		return queueFamilyProperties;
	}

	ArrayList<VkExtensionProperties> VulkanPhysicalDeviceInfo::extensions() const {
		uint32_t count;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
		ArrayList<VkExtensionProperties> extensionProperties(count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensionProperties.data());
		return extensionProperties;
	}

	ArrayList<VkLayerProperties> VulkanPhysicalDeviceInfo::layers() const {
		uint32_t count;
		vkEnumerateDeviceLayerProperties(physicalDevice, &count, nullptr);
		ArrayList<VkLayerProperties> layerProperties(count);
		vkEnumerateDeviceLayerProperties(physicalDevice, &count, layerProperties.data());
		return layerProperties;
	}

	VkPhysicalDeviceLimits VulkanPhysicalDeviceInfo::limits() const {
		return properties().limits;
	}

	bool RankedDevice::operator<(const RankedDevice &rhs) const {
		return score < rhs.score;
	}

	bool RankedDevice::operator>(const RankedDevice &rhs) const {
		return score > rhs.score;
	}

	bool RankedDevice::operator<=(const RankedDevice &rhs) const {
		return score <= rhs.score;
	}

	bool RankedDevice::operator>=(const RankedDevice &rhs) const {
		return score >= rhs.score;
	}
}