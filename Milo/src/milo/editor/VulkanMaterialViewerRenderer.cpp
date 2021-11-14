#include "milo/editor/VulkanMaterialViewerRenderer.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanMaterialViewerRenderer::VulkanMaterialViewerRenderer() {

		VulkanFramebuffer::CreateInfo createInfo{};
		createInfo.size = {1920, 1080};
		createInfo.colorAttachments.push_back(PixelFormat::RGBA32F);
		createInfo.depthAttachments.push_back(PixelFormat::DEPTH);

		VulkanFramebuffer::ApiInfo apiInfo{};
		apiInfo.device = VulkanContext::get()->device();
		createInfo.apiInfo = &apiInfo;

		m_Framebuffer = new VulkanFramebuffer(createInfo);

		m_Camera.position = Vector4(0, 0, -10, 1);
		m_Camera.view = glm::lookAt(Vector3(m_Camera.position), Vector3(0, 0, 0), Vector3(0, 1, 0));
		m_Camera.proj = glm::perspective(radians(45.0f), m_Framebuffer->size().aspect(), 0.1f, 20.0f);
		m_Camera.viewProjection = m_Camera.proj * m_Camera.view;

		m_MaterialPBRRenderer = new VulkanMaterialPBRRenderer(m_Framebuffer, &m_Camera);
		m_MaterialSkyboxRenderer = new VulkanMaterialSkyboxRenderer(m_Framebuffer, &m_Camera);
	}

	VulkanMaterialViewerRenderer::~VulkanMaterialViewerRenderer() {
		DELETE_PTR(m_MaterialSkyboxRenderer);
		DELETE_PTR(m_MaterialPBRRenderer);
		DELETE_PTR(m_Framebuffer);
	}

	const Texture2D& VulkanMaterialViewerRenderer::render(Material* material) {

		VulkanTexture2D* texture = (VulkanTexture2D*) m_Framebuffer->colorAttachments()[0];

		texture->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_MaterialPBRRenderer->render(material);
		m_MaterialSkyboxRenderer->render();

		texture->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		return *texture;
	}
}