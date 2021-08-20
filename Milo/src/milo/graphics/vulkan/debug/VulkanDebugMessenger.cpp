#include "milo/graphics/vulkan/debug/VulkanDebugMessenger.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include <regex>

#ifdef _DEBUG
#define THROW_EXCEPTION_ON_ERROR
#endif

namespace milo {

	VkResult createDebugMessenger(VkInstance instance,
								  const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
								  VkDebugUtilsMessengerEXT* debugMessenger) {
		auto vkFunction = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		return vkFunction != nullptr ? vkFunction(instance, createInfo, nullptr, debugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger) {
		auto vkFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if(vkFunction == nullptr)
			throw MILO_RUNTIME_EXCEPTION("Failed to find function vkDestroyDebugUtilsMessengerEXT");
		vkFunction(instance, debugMessenger, nullptr);
	}

	static inline const char* messageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType) {
		switch(messageType) {
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				return "PERFORMANCE";
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				return "VALIDATION";
			default:
				return "GENERAL";
		}
	}

	static inline bool isLoaderMessage(String&& pMessageIdName) {
		return pMessageIdName.find("Loader Message") != String::npos;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
												 VkDebugUtilsMessageTypeFlagsEXT messageType,
												 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
												 void* userData) {

		if(messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && isLoaderMessage(callbackData->pMessageIdName)) return VK_FALSE;

		VulkanAPICall apiCall = VulkanAPICallManager::popVkCall();

		String message = String("[VULKAN][")
				.append(messageTypeToString(messageType))
				.append("]: (")
				.append(callbackData->pMessageIdName).append("): ")
				.append(callbackData->pMessage)
				.append("\n\tVulkan call: ").append(apiCall.function)
				.append("\nStackTrace:\n").append(str(apiCall.stacktrace));

		switch(messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				//Log::trace(message);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				//Log::debug(message);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				Log::warn(message);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
#ifdef THROW_EXCEPTION_ON_ERROR
				throw MILO_RUNTIME_EXCEPTION(message);
#endif
				Log::error(message);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
				break;
		}

		return VK_FALSE;
	}

	VulkanDebugMessenger::VulkanDebugMessenger(VulkanContext* context) : m_Context(context) {
		VkDebugUtilsMessengerCreateInfoEXT createInfo = getDebugMessengerCreateInfo();
		VK_CALL(createDebugMessenger(context->vkInstance(), &createInfo, &m_VkDebugMessenger));
	}

	VulkanDebugMessenger::~VulkanDebugMessenger() {
		destroyDebugMessenger(m_Context->vkInstance(), m_VkDebugMessenger);
		m_VkDebugMessenger = VK_NULL_HANDLE;
	}

	VkDebugUtilsMessengerEXT VulkanDebugMessenger::vkDebugUtilsMessenger() const {
		return m_VkDebugMessenger;
	}

	VkDebugUtilsMessengerCreateInfoEXT VulkanDebugMessenger::getDebugMessengerCreateInfo() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		return createInfo;
	}

}