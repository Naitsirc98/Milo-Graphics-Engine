#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanSkyboxFactory::VulkanSkyboxFactory() {
		m_Device = VulkanContext::get()->device();
		m_EnvironmentPass = new VulkanEnvironmentMapPass(m_Device);
		m_IrradiancePass = new VulkanIrradianceMapPass(m_Device);
		m_PrefilterPass = new VulkanPrefilterMapPass(m_Device);
		m_BRDFPass = new VulkanBRDFMapPass(m_Device);
		m_PreethamSkyPass = new VulkanPreethamSkyEnvironmentPass(m_Device);
	}

	VulkanSkyboxFactory::~VulkanSkyboxFactory() {
		DELETE_PTR(m_EnvironmentPass);
		DELETE_PTR(m_IrradiancePass);
		DELETE_PTR(m_PrefilterPass);
		DELETE_PTR(m_BRDFPass);
	}

	Skybox* VulkanSkyboxFactory::create(const String& name, const String& imageFile, const SkyboxLoadInfo& loadInfo) {

		VulkanTexture2D* equirectangularTexture = createEquirectangularTexture(imageFile);
		VulkanCubemap* environmentMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanCubemap* irradianceMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanCubemap* prefilterMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanTexture2D* brdfMap = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);

		VulkanSkyboxPassExecuteInfo execInfo{};
		execInfo.equirectangularTexture = equirectangularTexture;
		execInfo.environmentMap = environmentMap;
		execInfo.irradianceMap = irradianceMap;
		execInfo.prefilterMap = prefilterMap;
		execInfo.brdfMap = brdfMap;
		execInfo.loadInfo = &loadInfo;

		m_Device->computeCommandPool()->execute([&](VkCommandBuffer commandBuffer) {
			execInfo.commandBuffer = commandBuffer;
			m_EnvironmentPass->execute(execInfo);
			m_IrradiancePass->execute(execInfo);
			m_PrefilterPass->execute(execInfo);
			m_BRDFPass->execute(execInfo);
		});

		Skybox* skybox = new Skybox(name, imageFile);
		skybox->m_EquirectangularTexture = Ref<VulkanTexture2D>(equirectangularTexture);
		skybox->m_EnvironmentMap = environmentMap;
		skybox->m_IrradianceMap = irradianceMap;
		skybox->m_BRDFMap = brdfMap;
		skybox->m_PrefilterMap = prefilterMap;
		skybox->m_PrefilterLODBias = loadInfo.lodBias;
		skybox->m_MaxPrefilterLOD = loadInfo.maxLOD;

		Assets::textures().addIcon(name, skybox->equirectangularTexture());

		equirectangularTexture->setName(name + "_EquirectangularTexture");
		environmentMap->setName(name + "_EnvironmentMap");
		irradianceMap->setName(name + "_IrradianceMap");
		prefilterMap->setName(name + "_PrefilterMap");
		brdfMap->setName(name + "_BRDFMap");

		return skybox;
	}

	VulkanTexture2D* VulkanSkyboxFactory::createEquirectangularTexture(const String& imageFile) {

		Image* image = Image::loadImage(imageFile, PixelFormat::RGBA32F);

		VulkanTexture2D* texture = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT);

		Texture2D::AllocInfo allocInfo{};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();
		allocInfo.mipLevels = 1;

		texture->allocate(allocInfo);

		VkSamplerCreateInfo samplerInfo = mvk::SamplerCreateInfo::create();
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		texture->vkSampler(VulkanContext::get()->samplerMap()->get(samplerInfo));

		DELETE_PTR(image);

		return texture;
	}

	PreethamSky* VulkanSkyboxFactory::createPreethamSky(const String& name, const SkyboxLoadInfo& loadInfo,
														float turbidity, float azimuth, float inclination) {

		VulkanCubemap* environmentMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanCubemap* irradianceMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanCubemap* prefilterMap = VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);
		VulkanTexture2D* brdfMap = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_STORAGE_BIT);

		VulkanSkyboxPassExecuteInfo execInfo{};
		execInfo.environmentMap = environmentMap;
		execInfo.irradianceMap = irradianceMap;
		execInfo.prefilterMap = prefilterMap;
		execInfo.brdfMap = brdfMap;
		execInfo.loadInfo = &loadInfo;
		execInfo.turbidity = turbidity;
		execInfo.azimuth = azimuth;
		execInfo.inclination = inclination;

		m_Device->computeCommandPool()->execute([&](VkCommandBuffer commandBuffer) {
			execInfo.commandBuffer = commandBuffer;
			m_PreethamSkyPass->execute(execInfo);
			m_IrradiancePass->execute(execInfo);
			m_PrefilterPass->execute(execInfo);
			m_BRDFPass->execute(execInfo);
		});

		PreethamSky* sky = new PreethamSky(name);
		sky->m_EnvironmentMap = environmentMap;
		sky->m_IrradianceMap = irradianceMap;
		sky->m_PrefilterMap = prefilterMap;
		sky->m_BRDFMap = brdfMap;
		sky->m_PrefilterLODBias = loadInfo.lodBias;
		sky->m_MaxPrefilterLOD = loadInfo.maxLOD;
		sky->turbidity(turbidity)->azimuth(azimuth)->inclination(inclination);
		sky->m_Dirty = false;

		environmentMap->setName(name + "_EnvironmentMap");
		irradianceMap->setName(name + "_IrradianceMap");
		prefilterMap->setName(name + "_PrefilterMap");
		brdfMap->setName(name + "_BRDFMap");

		return sky;
	}

	void VulkanSkyboxFactory::updatePreethamSky(PreethamSky* sky) {

		SkyboxLoadInfo loadInfo{};
		loadInfo.environmentMapSize = sky->environmentMap()->width();
		loadInfo.irradianceMapSize = sky->irradianceMap()->width();
		loadInfo.prefilterMapSize = sky->prefilterMap()->width();
		loadInfo.brdfSize = sky->brdfMap()->width();
		loadInfo.lodBias = sky->m_PrefilterLODBias;
		loadInfo.maxLOD = sky->m_MaxPrefilterLOD;

		VulkanSkyboxPassExecuteInfo execInfo{};
		execInfo.environmentMap = dynamic_cast<VulkanCubemap*>(sky->environmentMap());
		execInfo.irradianceMap = dynamic_cast<VulkanCubemap*>(sky->irradianceMap());
		execInfo.prefilterMap = dynamic_cast<VulkanCubemap*>(sky->prefilterMap());
		execInfo.brdfMap = dynamic_cast<VulkanTexture2D*>(sky->brdfMap());
		execInfo.loadInfo = &loadInfo;
		execInfo.turbidity = sky->turbidity();
		execInfo.azimuth = sky->azimuth();
		execInfo.inclination = sky->inclination();

		m_Device->computeCommandPool()->execute([&](VkCommandBuffer commandBuffer) {
			execInfo.commandBuffer = commandBuffer;
			m_PreethamSkyPass->execute(execInfo);
			m_IrradiancePass->execute(execInfo);
			m_PrefilterPass->execute(execInfo);
			m_BRDFPass->execute(execInfo);
		});

		sky->m_Dirty = false;
	}
}