#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/VulkanFormats.h"
#include <algorithm>
#include <utility>

namespace milo {

	void VulkanQueue::submit(uint32_t submitCount, VkSubmitInfo* submitInfos, VkFence fence) {
		VK_CALL(vkQueueSubmit(m_VkQueue, submitCount, submitInfos, fence));
	}

	void VulkanQueue::awaitTermination() {
		m_Device->awaitTermination(m_VkQueue);
	}

	// =====

	VulkanDevice::VulkanDevice(VulkanContext* context) : m_Context(context) {
	}

	VulkanDevice::~VulkanDevice() {
		awaitTermination();

		DELETE_PTR(m_GraphicsCommandPool);
		DELETE_PTR(m_ComputeCommandPool);
		DELETE_PTR(m_TransferCommandPool);

		vkDestroyDevice(m_Logical, nullptr);
		m_Logical = VK_NULL_HANDLE;
		m_Physical = VK_NULL_HANDLE;
	}

	void VulkanDevice::init(const VulkanDevice::Info& info) {

		m_Physical = info.physicalDevice;

		m_GraphicsQueue.init(this, "GraphicsQueue", VK_QUEUE_GRAPHICS_BIT);
		m_ComputeQueue.init(this, "ComputeQueue", VK_QUEUE_COMPUTE_BIT);
		m_TransferQueue.init(this, "TransferQueue", VK_QUEUE_TRANSFER_BIT);
		m_PresentationQueue.init(this, "PresentationQueue", VK_QUEUE_GRAPHICS_BIT);

		VulkanPhysicalDeviceInfo physicalDeviceInfo(m_Physical);
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

		// TODO: should check for support
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDepthStencilLayoutsFeatures = {};
		separateDepthStencilLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;
		separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts = VK_TRUE;

		createInfo.pNext = &separateDepthStencilLayoutsFeatures;

		VK_CALL(vkCreateDevice(m_Physical, &createInfo, nullptr, &m_Logical));

		getQueues();

		m_GraphicsCommandPool = new VulkanCommandPool(&m_GraphicsQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_ComputeCommandPool = new VulkanCommandPool(&m_ComputeQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		m_TransferCommandPool = new VulkanCommandPool(&m_TransferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	}

	void VulkanDevice::awaitTermination() {
		vkDeviceWaitIdle(m_Logical);
	}

	void VulkanDevice::awaitTermination(VkQueue queue) {
		vkQueueWaitIdle(queue);
	}

	VulkanContext* VulkanDevice::context() const {
		return m_Context;
	}

	VkPhysicalDevice VulkanDevice::physical() const {
		return m_Physical;
	}

	VkDevice VulkanDevice::logical() const {
		return m_Logical;
	}

	VulkanQueue* VulkanDevice::graphicsQueue() {
		return &m_GraphicsQueue;
	}

	VulkanQueue* VulkanDevice::computeQueue() {
		return &m_ComputeQueue;
	}

	VulkanQueue* VulkanDevice::transferQueue() {
		return &m_TransferQueue;
	}

	VulkanQueue* VulkanDevice::presentationQueue() {
		return &m_PresentationQueue;
	}

	VulkanQueue* VulkanDevice::queue(VkQueueFlags type) {
		switch(type) {
			case VK_QUEUE_GRAPHICS_BIT: return &m_GraphicsQueue;
			case VK_QUEUE_COMPUTE_BIT: return &m_ComputeQueue;
			case VK_QUEUE_TRANSFER_BIT: return &m_TransferQueue;
		}
		throw MILO_RUNTIME_EXCEPTION(str("Unsupported queue type ") + str(type));
	}

	VulkanPhysicalDeviceInfo VulkanDevice::info() const {
		return VulkanPhysicalDeviceInfo(m_Physical);
	}

	String VulkanDevice::name() const {
		return info().properties().deviceName;
	}

	VkFormat VulkanDevice::depthFormat() const {
		return VulkanFormats::findDepthFormat(m_Physical);
	}

	VulkanCommandPool* VulkanDevice::graphicsCommandPool() const {
		return m_GraphicsCommandPool;
	}

	VulkanCommandPool* VulkanDevice::computeCommandPool() const {
		return m_ComputeCommandPool;
	}

	VulkanCommandPool* VulkanDevice::transferCommandPool() const {
		return m_TransferCommandPool;
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

		VulkanQueue* queue = this->queue(queueType);
		queue->m_Family = bestQueueFamily;
		queue->m_Index = 0;

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

		VkSurfaceKHR surface = m_Context->windowSurface()->vkSurface();

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

		m_PresentationQueue.m_Family = bestQueueFamily;
		m_PresentationQueue.m_Index = 0;

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

	void VulkanDevice::getQueues() {
		vkGetDeviceQueue(m_Logical, m_GraphicsQueue.m_Family, m_GraphicsQueue.m_Index, &m_GraphicsQueue.m_VkQueue);
		vkGetDeviceQueue(m_Logical, m_TransferQueue.m_Family, m_TransferQueue.m_Index, &m_TransferQueue.m_VkQueue);
		vkGetDeviceQueue(m_Logical, m_ComputeQueue.m_Family, m_ComputeQueue.m_Index, &m_ComputeQueue.m_VkQueue);
		vkGetDeviceQueue(m_Logical, m_PresentationQueue.m_Family, m_PresentationQueue.m_Index, &m_PresentationQueue.m_VkQueue);
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

	bool VulkanDevice::operator==(const VulkanDevice& rhs) const {
		return m_Physical == rhs.m_Physical && m_Logical == rhs.m_Logical;
	}

	bool VulkanDevice::operator!=(const VulkanDevice& rhs) const {
		return ! (rhs == *this);
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

	uint32_t VulkanPhysicalDeviceInfo::uniformBufferAlignment() const {
		return properties().limits.minUniformBufferOffsetAlignment;
	}

	uint32_t VulkanPhysicalDeviceInfo::storageBufferAlignment() const {
		return properties().limits.minStorageBufferOffsetAlignment;
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