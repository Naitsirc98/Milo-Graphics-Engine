#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanSkyboxFactory::VulkanSkyboxFactory() {
		m_Device = VulkanContext::get()->device();
		m_EnvironmentPass = new VulkanEnvironmentMapPass(m_Device);
		//m_IrradiancePass = new VulkanIrradianceMapPass(m_Device);
		//m_PrefilterPass = new VulkanPrefilterMapPass(m_Device);
		//m_BRDFPass = new VulkanBRDFPass(m_Device);
	}

	VulkanSkyboxFactory::~VulkanSkyboxFactory() {
		DELETE_PTR(m_EnvironmentPass);
		//DELETE_PTR(m_IrradiancePass);
		//DELETE_PTR(m_PrefilterPass);
		//DELETE_PTR(m_BRDFPass);
	}

	Skybox* VulkanSkyboxFactory::create(const String& name, const String& imageFile, const SkyboxLoadInfo& loadInfo) {

		VulkanTexture2D* equirectangularTexture = createEquirectangularTexture(imageFile);

		VulkanCubemap* environmentMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		//VulkanCubemap* irradianceMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		//VulkanCubemap* prefilterMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		//VulkanTexture2D* brdfMap = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);

		VulkanTask task{};
		task.run = [&](VkCommandBuffer commandBuffer) {

			VulkanSkyboxPassExecuteInfo execInfo{};
			execInfo.equirectangularTexture = equirectangularTexture;
			execInfo.environmentMap = environmentMap;
			//execInfo.irradianceMap = irradianceMap;
			//execInfo.prefilterMap = prefilterMap;
			//execInfo.brdfMap = brdfMap;
			execInfo.loadInfo = &loadInfo;
			execInfo.commandBuffer = commandBuffer;

			m_EnvironmentPass->execute(execInfo); // TODO
			//m_IrradiancePass->execute(execInfo);
			//m_PrefilterPass->execute(execInfo);
			//m_BRDFPass->execute(execInfo);
		};

		m_Device->computeCommandPool()->execute(task);

		Skybox* skybox = new Skybox(name, imageFile);
		skybox->m_EnvironmentMap = environmentMap;
		//skybox->m_IrradianceMap = irradianceMap;
		//skybox->m_PrefilterMap = prefilterMap;
		//skybox->m_BRDFMap = brdfMap;
		skybox->m_PrefilterLODBias = loadInfo.lodBias;
		skybox->m_MaxPrefilterLOD = loadInfo.maxLOD;

		return skybox;
	}

	VulkanTexture2D* VulkanSkyboxFactory::createEquirectangularTexture(const String& imageFile) {

		Image* image = Image::loadImage(imageFile, PixelFormat::RGBA16F);

		VulkanTexture2D* texture = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT);

		Texture2D::AllocInfo allocInfo{};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();
		allocInfo.mipLevels = 1;

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		return texture;
	}
}