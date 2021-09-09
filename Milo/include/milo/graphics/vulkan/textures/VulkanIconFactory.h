#pragma once

#include "milo/assets/textures/TextureManager.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	class VulkanIconFactory : public IconFactory {
		friend class TextureManager;
	private:
		VulkanDevice* m_Device{nullptr};
		VkRenderPass m_RenderPass{VK_NULL_HANDLE};
		VulkanTexture2D* m_DepthTexture{nullptr};
		VulkanGraphicsPipeline* m_GraphicsPipeline{nullptr};
	private:
		VulkanIconFactory();
		~VulkanIconFactory();
	public:
		Texture2D* createIcon(Mesh* mesh, Material* material, const Size& size) override;
	private:
		// Called once
		void createRenderPass();
		void createDepthTexture(const Size& size);
		void createGraphicsPipeline();
		// Called every time
		static VulkanTexture2D* createTexture2D(const Size& size);
		VkFramebuffer createFramebuffer(VulkanTexture2D* colorAttachment, const Size& size);
	};
}