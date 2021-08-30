#pragma once

#include "milo/assets/skybox/SkyboxFactory.h"
#include "milo/io/Files.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"

namespace milo {

	struct VulkanSkyboxPassExecuteInfo {
		VulkanTexture2D* equirectangularTexture;
		VulkanCubemap* environmentMap;
		VulkanCubemap* irradianceMap;
		VulkanCubemap* prefilterMap;
		VulkanTexture2D* brdfMap;
		const SkyboxLoadInfo* loadInfo;
		VkCommandBuffer commandBuffer;
	};

	class VulkanEnvironmentMapPass {
	private:
		VulkanDevice* m_Device;
		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};
	public:
		VulkanEnvironmentMapPass(VulkanDevice* device);
		~VulkanEnvironmentMapPass();
		void execute(const VulkanSkyboxPassExecuteInfo& execInfo);
	private:
		void updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createPipelineLayout();
		void createComputePipeline();
	};

	class VulkanIrradianceMapPass {
	private:
		VulkanDevice* m_Device;
		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};
	public:
		VulkanIrradianceMapPass(VulkanDevice* device);
		~VulkanIrradianceMapPass();
		void execute(const VulkanSkyboxPassExecuteInfo& execInfo);
	private:
		void updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createPipelineLayout();
		void createComputePipeline();
	};

	class VulkanPrefilterMapPass {
	private:
		VulkanDevice* m_Device;
		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};
	public:
		VulkanPrefilterMapPass(VulkanDevice* device);
		~VulkanPrefilterMapPass();
		void execute(const VulkanSkyboxPassExecuteInfo& execInfo);
	private:
		void updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createPipelineLayout();
		void createComputePipeline();
	};

	class VulkanBRDFPass {
	private:
		VulkanDevice* m_Device;
		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};
	public:
		VulkanBRDFPass(VulkanDevice* device);
		~VulkanBRDFPass();
		void execute(const VulkanSkyboxPassExecuteInfo& execInfo);
	private:
		void updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createPipelineLayout();
		void createComputePipeline();
	};

	class VulkanSkyboxFactory : public SkyboxFactory {
		friend class SkyboxFactory;
	private:
		VulkanDevice* m_Device{nullptr};
		VulkanEnvironmentMapPass* m_EnvironmentPass{nullptr};
		VulkanIrradianceMapPass* m_IrradiancePass{nullptr};
		VulkanPrefilterMapPass* m_PrefilterPass{nullptr};
		VulkanBRDFPass* m_BRDFPass{nullptr};
	private:
		VulkanSkyboxFactory();
		~VulkanSkyboxFactory() override;
	public:
		Skybox* create(const String& name, const String& imageFile, const SkyboxLoadInfo& loadInfo = DEFAULT_SKYBOX_LOAD_INFO) override;
	private:
		VulkanTexture2D* createEquirectangularTexture(const String imageFile);
	};
}