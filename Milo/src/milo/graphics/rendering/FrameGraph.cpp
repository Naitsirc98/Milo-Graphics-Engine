#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"

namespace milo {

	FrameGraph::FrameGraph() {
		m_RenderPasses.reserve(64);
		m_RenderPassExecutionList.reserve(m_RenderPasses.capacity());
	}

	FrameGraph::~FrameGraph() {
		for(RenderPass* renderPass : m_RenderPasses) {
			DELETE_PTR(renderPass);
		}
	}

	void FrameGraph::init(FrameGraphResourcePool* resourcePool) {
		m_ResourcePool = resourcePool;
	}

	void FrameGraph::setup(Scene* scene) {

		m_ResourcePool->update();
		m_ResourcePool->clearReferences();

		push<GeometryRenderPass>();

		//push<PBRForwardRenderPass>();

		if(scene->skybox() != nullptr) {
			push<SkyboxRenderPass>();
		}

		push<FinalRenderPass>();
	}

	void FrameGraph::compile(Scene* scene) {
		assignResources();
		deleteUnusedRenderPasses();
		freeUnusedResources();
	}

	void FrameGraph::execute(Scene* scene) {

		for(RenderPass* renderPass : m_RenderPassExecutionList) {
			renderPass->execute(scene);
		}
		m_RenderPassExecutionList.clear();
	}

	template<typename T>
	inline static uint32_t assignResource(T& dstResource, const T& srcResource) {
		if(dstResource != srcResource) {
			dstResource = srcResource;
			return 1;
		}
		return 0;
	}

	inline void FrameGraph::assignResources() {

		for(RenderPass* pass : m_RenderPassExecutionList) {

			RenderPass::InputDescription inputDesc = pass->inputDescription();
			RenderPass::OutputDescription outputDesc = pass->outputDescription();

			uint32_t changes = 0;

			// Input resources
			for(uint8_t i = 0; i < inputDesc.bufferCount; ++i) {
				changes += assignResource(pass->m_Input.buffers[i], m_ResourcePool->getBuffer(inputDesc.buffers[i]));
			}
			for(uint8_t i = 0; i < inputDesc.textureCount; ++i) {
				changes += assignResource(pass->m_Input.textures[i], m_ResourcePool->getTexture2D(inputDesc.textures[i]));
			}

			// Output resources
			for(uint8_t i = 0; i < outputDesc.bufferCount; ++i) {
				changes += assignResource(pass->m_Output.buffers[i], m_ResourcePool->getBuffer(outputDesc.buffers[i]));
			}
			for(uint8_t i = 0; i < outputDesc.textureCount; ++i) {
				changes += assignResource(pass->m_Output.textures[i], m_ResourcePool->getTexture2D(outputDesc.textures[i]));
			}

			if(changes > 0) {
#ifdef _DEBUG
				Log::debug("Compiling render pass {}", pass->name());
#endif
				pass->compile(m_ResourcePool);
			}
		}
	}

	inline void FrameGraph::deleteUnusedRenderPasses() {

		std::remove_if(m_RenderPasses.begin(), m_RenderPasses.end(), [&](RenderPass* renderPass) {

			for(RenderPass* activeRenderPass : m_RenderPassExecutionList) {
				if(activeRenderPass->getId() == renderPass->getId()) return false;
			}

			DELETE_PTR(renderPass);
			return true;
		});
	}

	inline void FrameGraph::freeUnusedResources() {
		m_ResourcePool->freeUnreferencedResources();
	}

	// ====

	FrameGraphResourcePool* FrameGraphResourcePool::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFrameGraphResourcePool();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}